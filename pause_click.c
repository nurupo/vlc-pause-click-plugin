/*****************************************************************************
 * pause_click.c : A filter that allows to pause/play a video by a mouse click
 *****************************************************************************
 * Copyright (C) 2014-2019 Maxim Biro
 *
 * Authors: Maxim Biro <nurupo.contributions@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define N_(str) (str)
#endif

#include <vlc/libvlc_version.h>

#if LIBVLC_VERSION_MAJOR >= 3
# define VLC_MODULE_LICENSE VLC_LICENSE_LGPL_2_1_PLUS
# define VLC_MODULE_COPYRIGHT "Copyright (C) 2014-2019 Maxim Biro"
#endif

#include <vlc_atomic.h>
#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_input.h>
#include <vlc_mouse.h>
#if LIBVLC_VERSION_MAJOR >= 4
# include <vlc_player.h>
#endif
#include <vlc_playlist.h>
#include <vlc_plugin.h>
#if LIBVLC_VERSION_MAJOR == 2
# include <vlc_spu.h>
#endif
#include <vlc_threads.h>
#if LIBVLC_VERSION_MAJOR >= 3
# include <vlc_vout.h>
#endif
#include <vlc_vout_osd.h>

#if LIBVLC_VERSION_MAJOR == 2 && LIBVLC_VERSION_MINOR == 1
# include "vlc_interface-2.1.0-git.h"
#elif LIBVLC_VERSION_MAJOR == 2 && LIBVLC_VERSION_MINOR == 2
# include "vlc_interface-2.2.0-git.h"
#elif LIBVLC_VERSION_MAJOR >= 3 && LIBVLC_VERSION_MINOR >= 0
# include <vlc_interface.h>
#else
# error "VLC version " LIBVLC_VERSION_MAJOR "." LIBVLC_VERSION_MINOR " is too old and won't be supported"
#endif

#define UNUSED(x) (void)(x)

static const char *const mouse_button_names[] = {N_("None"), N_("Left Button"), N_("Middle Button"), N_("Right Button"), N_("Scroll Up"), N_("Scroll Down"), N_("Scroll Left"), N_("Scroll Right")};
static const int mouse_button_values_index[] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int mouse_button_values[] = {-1, MOUSE_BUTTON_LEFT, MOUSE_BUTTON_CENTER, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_WHEEL_UP, MOUSE_BUTTON_WHEEL_DOWN, MOUSE_BUTTON_WHEEL_LEFT, MOUSE_BUTTON_WHEEL_RIGHT};

#define CFG_PREFIX "pause-click-"

#define MOUSE_BUTTON_CFG CFG_PREFIX "mouse-button"
#define MOUSE_BUTTON_DEFAULT 1 // MOUSE_BUTTON_LEFT

#define IGNORE_DOUBLE_CLICK_CFG CFG_PREFIX "ignore-double-click"
#define IGNORE_DOUBLE_CLICK_DEFAULT false

#define DOUBLE_CLICK_DELAY_CFG CFG_PREFIX "double-click-delay"
#define DOUBLE_CLICK_DELAY_DEFAULT 300

#define DISABLE_FS_TOGGLE_CFG CFG_PREFIX "disable-fs-toggle"
#define DISABLE_FS_TOGGLE_DEFAULT false

#define FS_TOGGLE_MOUSE_BUTTON_CFG CFG_PREFIX "fs-toggle-mouse-button"
#define FS_TOGGLE_MOUSE_BUTTON_DEFAULT 0 // None

#define DISABLE_CONTEXT_MENU_TOGGLE_CFG CFG_PREFIX "disable-context-menu-toggle"
#define DISABLE_CONTEXT_MENU_TOGGLE_DEFAULT false

#define CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_CFG CFG_PREFIX "context-menu-toggle-mouse-button"
#define CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_DEFAULT 0 // None

#define DISPLAY_ICON_CFG CFG_PREFIX "display-icon"
#define DISPLAY_ICON_DEFAULT true

static int OpenFilter(vlc_object_t *);
static void CloseFilter(vlc_object_t *);
static int OpenInterface(vlc_object_t *);
static void CloseInterface(vlc_object_t *);


static intf_thread_t *p_intf = NULL;

static vlc_timer_t timer;
static bool timer_initialized = false;
static atomic_bool timer_scheduled;


vlc_module_begin()
    set_description(N_("Pause/Play video on mouse click"))
    set_shortname(N_("Pause click"))
#if LIBVLC_VERSION_MAJOR == 2
    set_capability("video filter2", 0)
#elif LIBVLC_VERSION_MAJOR >= 3
    set_capability("video filter", 0)
#endif
    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VFILTER)
    set_callbacks(OpenFilter, CloseFilter)
    add_integer(MOUSE_BUTTON_CFG, MOUSE_BUTTON_DEFAULT, N_("Mouse button"),
                N_("Defines the mouse button that will pause/play the video."), false)
    vlc_config_set(VLC_CONFIG_LIST, (size_t)(sizeof(mouse_button_values_index)/sizeof(int))-1,
                   mouse_button_values_index+1, mouse_button_names+1);
    add_bool(IGNORE_DOUBLE_CLICK_CFG, IGNORE_DOUBLE_CLICK_DEFAULT,
             N_("Ignore double clicks"), N_("Useful if you don't want the video to "
             "pause when double clicking to fullscreen. Note that enabling this "
             "will delay pause/play action by the double click interval, so the "
             "experience might not be as snappy as with this option disabled."), false)
    add_integer_with_range(DOUBLE_CLICK_DELAY_CFG, DOUBLE_CLICK_DELAY_DEFAULT,
                           20, 5000, N_("Double click interval (milliseconds)"),
                           N_("Two clicks made during this time interval will be "
                           "treated as a double click and will be ignored."), false)
    add_bool(DISABLE_FS_TOGGLE_CFG, DISABLE_FS_TOGGLE_DEFAULT,
             N_("Disable fullscreen toggle on double click"),
             N_("The video will no longer fullscreen if you double click on it. "
             "This option is unaffected by the double click interval option."), false)
    add_integer(FS_TOGGLE_MOUSE_BUTTON_CFG, FS_TOGGLE_MOUSE_BUTTON_DEFAULT,
                N_("Assign fullscreen toggle to"),
                N_("Assigns fullscreen toggle to a mouse button."), false)
    change_integer_list(mouse_button_values_index, mouse_button_names)
    add_bool(DISABLE_CONTEXT_MENU_TOGGLE_CFG, DISABLE_CONTEXT_MENU_TOGGLE_DEFAULT,
             N_("Disable context menu toggle on right click"),
             N_("The context menu will no longer pop up if you right click on the video. "
             "Useful if you want to pause/play or full screen on right click."), false)
    add_integer(CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_CFG, CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_DEFAULT,
                N_("Assign context menu toggle to"),
                N_("Assigns context menu toggle to a mouse button."), false)
    change_integer_list(mouse_button_values_index, mouse_button_names)
    add_bool(DISPLAY_ICON_CFG, DISPLAY_ICON_DEFAULT,
             N_("Display pause and play icons on the video"),
             N_("Overlay pause (\xE2\x96\xB6) and play (\xE2\x8F\xB8) icons "
             "on the video when it's paused and played respectively."), false)
        add_submodule()
        set_capability("interface", 0)
        set_category(CAT_INTERFACE)
        set_subcategory(SUBCAT_INTERFACE_CONTROL)
        set_callbacks(OpenInterface, CloseInterface)
vlc_module_end()


static bool is_in_menu(void) {
    if (!p_intf) {
        return false;
    }

#if 2 <= LIBVLC_VERSION_MAJOR && LIBVLC_VERSION_MAJOR <= 3
    playlist_t* p_playlist = pl_Get(p_intf);

    input_title_t* p_title = NULL;
    int i_title_id = -1;

    input_thread_t* p_input = playlist_CurrentInput(p_playlist);
    if (!p_input) {
        return false;
    }
    if (input_Control(p_input, INPUT_GET_TITLE_INFO, &p_title, &i_title_id) != VLC_SUCCESS) {
        vlc_object_release(p_input);
        return false;
    }
    vlc_object_release(p_input);

    if (!p_title) {
        return false;
    }
    if (
#if LIBVLC_VERSION_MAJOR == 2
        p_title->b_menu
#elif LIBVLC_VERSION_MAJOR >= 3
        p_title->i_flags & INPUT_TITLE_MENU || p_title->i_flags & INPUT_TITLE_INTERACTIVE
#endif
    ) {
        vlc_input_title_Delete(p_title);
        return true;
    }

    vlc_input_title_Delete(p_title);

    return false;
#elif LIBVLC_VERSION_MAJOR >= 4
    vlc_player_t* player = vlc_playlist_GetPlayer(vlc_intf_GetMainPlaylist(p_intf));
    vlc_player_Lock(player);

    const struct vlc_player_title* title = vlc_player_GetSelectedTitle(player);
    vlc_player_Unlock(player);
    if (!title) {
        return false;
    }
    if (title->flags & VLC_PLAYER_TITLE_MENU || title->flags & VLC_PLAYER_TITLE_INTERACTIVE) {
        return true;
    }

    return false;
#endif
}

static void display_icon(short icon) {
    if (!p_intf) {
        return;
    }

#if 2 <= LIBVLC_VERSION_MAJOR && LIBVLC_VERSION_MAJOR <= 3
    playlist_t* p_playlist = pl_Get(p_intf);

    input_thread_t* p_input = playlist_CurrentInput(p_playlist);
    if (!p_input) {
        return;
    }

    vout_thread_t** pp_vout;
    size_t i_vout;
    if (input_Control(p_input, INPUT_GET_VOUTS, &pp_vout, &i_vout) != VLC_SUCCESS) {
        vlc_object_release(p_input);
        return;
    }
    for (size_t i = 0; i < i_vout; i ++) {
        vout_OSDIcon(pp_vout[i],
#if LIBVLC_VERSION_MAJOR == 2
                     SPU_DEFAULT_CHANNEL,
#elif LIBVLC_VERSION_MAJOR >= 3
                     VOUT_SPU_CHANNEL_OSD,
#endif
                     icon);
        vlc_object_release((vlc_object_t *)pp_vout[i]);
    }
    vlc_object_release(p_input);
    free(pp_vout);
#elif LIBVLC_VERSION_MAJOR >= 4
    vlc_player_t* player = vlc_playlist_GetPlayer(vlc_intf_GetMainPlaylist(p_intf));
    vlc_player_Lock(player);

    vout_thread_t** pp_vout;
    size_t i_vout;
    pp_vout = vlc_player_vout_HoldAll(player, &i_vout);
    if (!pp_vout) {
        vlc_player_Unlock(player);
        return;
    }
    for (size_t i = 0; i < i_vout; i ++) {
        vout_OSDIcon(pp_vout[i], VOUT_SPU_CHANNEL_OSD, icon);
        vout_Release(pp_vout[i]);
    }

    vlc_player_Unlock(player);
    free(pp_vout);
#endif
}

static void pause_play(void)
{
    if (!p_intf) {
        return;
    }

    if (is_in_menu()) {
        return;
    }

#if 2 <= LIBVLC_VERSION_MAJOR && LIBVLC_VERSION_MAJOR <= 3
    playlist_t* p_playlist = pl_Get(p_intf);
    playlist_status_t status = playlist_Status(p_playlist);
    playlist_Control(p_playlist, status == PLAYLIST_RUNNING ? PLAYLIST_PAUSE : PLAYLIST_PLAY , 0);
    if (var_InheritBool(p_intf, DISPLAY_ICON_CFG)) {
        display_icon(status == PLAYLIST_RUNNING ? OSD_PAUSE_ICON : OSD_PLAY_ICON);
    }
#elif LIBVLC_VERSION_MAJOR >= 4
    vlc_player_t* player = vlc_playlist_GetPlayer(vlc_intf_GetMainPlaylist(p_intf));
    vlc_player_Lock(player);
    int state = vlc_player_GetState(player);
    state == VLC_PLAYER_STATE_PLAYING ? vlc_player_Pause(player) : vlc_player_Resume(player);
    vlc_player_Unlock(player);
    if (var_InheritBool(p_intf, DISPLAY_ICON_CFG)) {
        display_icon(state == VLC_PLAYER_STATE_PLAYING ? OSD_PAUSE_ICON : OSD_PLAY_ICON);
    }
#endif
}

static void timer_callback(void* data)
{
    UNUSED(data);

    if (!atomic_load(&timer_scheduled)) {
        return;
    }

    pause_play();

    atomic_store(&timer_scheduled, false);
}

static int cfg_get_mouse_button(vlc_object_t *p_obj, const char *cfg, int default_value) {
    int mouse_button = mouse_button_values[default_value];
    int mouse_button_index = var_InheritInteger(p_obj, cfg);
    if (mouse_button_index >= 0 && (size_t)mouse_button_index <= sizeof(mouse_button_values)-1) {
        mouse_button = mouse_button_values[mouse_button_index];
    }
    return mouse_button;
}

static int mouse(filter_t *p_filter, vlc_mouse_t *p_mouse_out, const vlc_mouse_t *p_mouse_old, const vlc_mouse_t *p_mouse_new)
{
    // we don't want to process anything if no mouse button was clicked
    if (p_mouse_new->i_pressed == 0 && !p_mouse_new->b_double_click) {
        return VLC_EGENERIC;
    }

    // get mouse button from settings. updates if user changes the setting
    const int mouse_button = cfg_get_mouse_button((vlc_object_t *)p_filter, MOUSE_BUTTON_CFG, MOUSE_BUTTON_DEFAULT);

    if (vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, mouse_button) ||
            // on some systems (e.g. Linux) b_double_click is not set for a double-click, so we track any click and
            // decide if it was a double click on our own. This provides the most uniform cross-platform behaviour.
            (p_mouse_new->b_double_click && mouse_button == MOUSE_BUTTON_LEFT)) {
        // if ignoring double click
        if (var_InheritBool(p_filter, IGNORE_DOUBLE_CLICK_CFG) && timer_initialized) {
            if (atomic_load(&timer_scheduled)) {
                // it's a double click -- cancel the scheduled pause/play, if any
                atomic_store(&timer_scheduled, false);
                vlc_timer_schedule(timer, false, 0, 0);
            } else {
                // it might be a single click -- schedule pause/play call
                atomic_store(&timer_scheduled, true);
                vlc_timer_schedule(timer, false, var_InheritInteger(p_filter, DOUBLE_CLICK_DELAY_CFG)*1000, 0);
            }
        } else {
            pause_play();
        }
    }

    bool filter = false;
    *p_mouse_out = *p_mouse_new;

    // prevent fullscreen from toggling on double click
    if (var_InheritBool(p_filter, DISABLE_FS_TOGGLE_CFG) && p_mouse_new->b_double_click) {
        p_mouse_out->b_double_click = 0;
        filter = true;
    }

    // toggle fullscreen on specified mouse click
    const int fs_mouse_button = cfg_get_mouse_button((vlc_object_t *)p_filter, FS_TOGGLE_MOUSE_BUTTON_CFG,
                                                     FS_TOGGLE_MOUSE_BUTTON_DEFAULT);
    if (fs_mouse_button != -1 && vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, fs_mouse_button)) {
        p_mouse_out->b_double_click = 1;
        filter = true;
    }

    // prevent the context menu from toggling on right click
    if (var_InheritBool(p_filter, DISABLE_CONTEXT_MENU_TOGGLE_CFG) &&
            vlc_mouse_IsPressed(p_mouse_new, MOUSE_BUTTON_RIGHT)) {
        p_mouse_out->i_pressed = 0;
        filter = true;
    }

    // toggle context menu on specified mouse click
    const int context_menu_mouse_button =
            cfg_get_mouse_button((vlc_object_t *)p_filter,CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_CFG,
                                 CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_DEFAULT);
    if (context_menu_mouse_button != -1 &&
            vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, context_menu_mouse_button)) {
        vlc_mouse_SetPressed(p_mouse_out, MOUSE_BUTTON_RIGHT);
        filter = true;
    }

    return filter ? VLC_SUCCESS : VLC_EGENERIC;
}

static picture_t *filter(filter_t *p_filter, picture_t *p_pic_in)
{
    UNUSED(p_filter);

    // don't alter picture
    return p_pic_in;
}

static int OpenFilter(vlc_object_t *p_this)
{
    filter_t *p_filter = (filter_t *)p_this;

    p_filter->pf_video_filter = filter;
    p_filter->pf_video_mouse = mouse;

    if (vlc_timer_create(&timer, &timer_callback, NULL)) {
        return VLC_EGENERIC;
    }
    timer_initialized = true;
    atomic_store(&timer_scheduled, false);

    return VLC_SUCCESS;
}

static void CloseFilter(vlc_object_t *p_this)
{
    UNUSED(p_this);

    if (timer_initialized) {
        vlc_timer_destroy(timer);
        timer_initialized = false;
        atomic_store(&timer_scheduled, false);
    }
}

static int OpenInterface(vlc_object_t *p_this)
{
    p_intf = (intf_thread_t*) p_this;

    return VLC_SUCCESS;
}

static void CloseInterface(vlc_object_t *p_this)
{
    UNUSED(p_this);

    p_intf = NULL;
}
