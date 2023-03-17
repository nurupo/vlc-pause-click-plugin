/*****************************************************************************
 * pause_click.c : A plugin that allows to pause/play a video by a mouse click
 *****************************************************************************
 * Copyright (C) 2014-2023 Maxim Biro
 *
 * Authors: Maxim Biro <nurupo.contributions@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
#include <inttypes.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define N_(str) (str)
#endif

#include <vlc/libvlc_version.h>
#include "version.h"

#if LIBVLC_VERSION_MAJOR >= 3
# define VLC_MODULE_LICENSE VLC_LICENSE_LGPL_2_1_PLUS
# define VLC_MODULE_COPYRIGHT VERSION_COPYRIGHT
#endif

#include <vlc_atomic.h>
#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_input.h>
#include <vlc_messages.h>
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
#include <vlc_vout.h>
#include <vlc_vout_osd.h>

#if LIBVLC_VERSION_MAJOR == 2 && LIBVLC_VERSION_MINOR == 1
# include "vlc_interface-2.1.0-git.h"
#elif LIBVLC_VERSION_MAJOR == 2 && LIBVLC_VERSION_MINOR == 2
# include "vlc_interface-2.2.0-git.h"
#elif LIBVLC_VERSION_MAJOR >= 3 && LIBVLC_VERSION_MINOR >= 0
# include <vlc_interface.h>
#else
# error "VLC version < 2.1 is not supported"
#endif

#define UNUSED(x) (void)(x)

static const char *const mouse_button_names[] = {N_("None"), N_("Left Button"), N_("Middle Button"), N_("Right Button"), N_("Scroll Up"), N_("Scroll Down"), N_("Scroll Left"), N_("Scroll Right")};
static const int mouse_button_values_index[] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int mouse_button_values[] = {-1, MOUSE_BUTTON_LEFT, MOUSE_BUTTON_CENTER, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_WHEEL_UP, MOUSE_BUTTON_WHEEL_DOWN, MOUSE_BUTTON_WHEEL_LEFT, MOUSE_BUTTON_WHEEL_RIGHT};

#define CFG_PREFIX "pause-click-"

#define MOUSE_BUTTON_CFG CFG_PREFIX "mouse-button"
#define MOUSE_BUTTON_DEFAULT 1 // MOUSE_BUTTON_LEFT

#define DOUBLE_CLICK_DELAY_CFG CFG_PREFIX "double-click-delay"
#define DOUBLE_CLICK_DELAY_DEFAULT 300

#define ENABLE_DOUBLE_CLICK_DELAY_CFG CFG_PREFIX "enable-double-click-delay"
#define ENABLE_DOUBLE_CLICK_DELAY_DEFAULT false

#define IGNORE_DOUBLE_CLICK_CFG CFG_PREFIX "ignore-double-click"
#define IGNORE_DOUBLE_CLICK_DEFAULT false

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

// VLC 4.0 removed the advanced flag in 3716a7da5ba8dc30dbd752227c6a893c71a7495b
#if LIBVLC_VERSION_MAJOR >= 4
#define _add_bool(name, v, text, longtext, advc) \
    add_bool(name, v, text, longtext)
#define _add_integer(name, value, text, longtext, advc) \
    add_integer(name, value, text, longtext)
#define _add_integer_with_range(name, value, i_min, i_max, text, longtext, advc) \
    add_integer_with_range(name, value, i_min, i_max, text, longtext)
#else
#define _add_bool add_bool
#define _add_integer add_integer
#define _add_integer_with_range add_integer_with_range
#endif

vlc_module_begin()
    set_description(N_("Pause/Play video on mouse click"))
    set_shortname(N_("Pause click"))
#if LIBVLC_VERSION_MAJOR == 2
    set_capability("video filter2", 0)
#elif LIBVLC_VERSION_MAJOR == 3
    set_capability("video filter", 0)
#endif
// VLC 4.0 removed categories and changed the way video filter callbacksare set
// 6f68f894986e11e3f6215f6c2c25e5c0a3139429 94e23d51bb91cff1c14ef1079193920f04f48fd1
#if LIBVLC_VERSION_MAJOR >= 4
    set_callback_video_filter(OpenFilter)
#else
    set_category(CAT_VIDEO)
    set_callbacks(OpenFilter, CloseFilter)
#endif
    set_subcategory(SUBCAT_VIDEO_VFILTER)
    set_help(N_("<style>"
                "p { margin:0.5em 0 0.5em 0; }"
                "</style>"
                "<p>"
                "v" VERSION_STRING "<br>"
                "Copyright " VERSION_COPYRIGHT
                "</p><p>"
                "Homepage: <a href=\"" VERSION_HOMEPAGE "\">" VERSION_HOMEPAGE "</a><br>"
                "Donate: <a href=\"https://www.paypal.com/donate?hosted_button_id=9HJHAH5UDL3GL\">PayPal</a>, "
                        "<a href=\"https://github.com/sponsors/nurupo\">GitHub Sponsors</a>, "
                        "<a href=\"bitcoin:34qxFsZjs1ZWVBwer11gXiycpv7QHTA8q3?label=nurupo&message=Donation+for+vlc-pause-click-plugin\">Bitcoin</a>"
                "</p>"))
    set_section(N_("General"), NULL)
    _add_integer(MOUSE_BUTTON_CFG, MOUSE_BUTTON_DEFAULT,
                 N_("Pause/play mouse button"),
                 N_("Defines the mouse button that will pause/play the video. "
                 "Note that the scroll buttons might not work, that's a bug in VLC."), false)
    vlc_config_set(VLC_CONFIG_LIST, (size_t)(sizeof(mouse_button_values_index)/sizeof(int))-1,
                   mouse_button_values_index+1, mouse_button_names+1);
    _add_bool(DISPLAY_ICON_CFG, DISPLAY_ICON_DEFAULT,
              N_("Show pause/play icon animations"),
              N_("Overlay pause and play icons on the video when it's paused and "
              "played respectively."), false)
    set_section(N_("Double click behavior"), NULL)
    _add_integer_with_range(DOUBLE_CLICK_DELAY_CFG, DOUBLE_CLICK_DELAY_DEFAULT,
                            20, 5000, N_("Custom double click interval (milliseconds)"),
                            N_("Two clicks made during this time interval will "
                            "be treated as a double click."), false)
    _add_bool(ENABLE_DOUBLE_CLICK_DELAY_CFG, ENABLE_DOUBLE_CLICK_DELAY_DEFAULT,
              N_("Enable the custom double click interval"),
              N_("Ignore system's double click interval and use our own instead. "
              "Useful when you want to make the time interval of double clicking "
              "to fullscreen be longer or shorter."), false)
    _add_bool(IGNORE_DOUBLE_CLICK_CFG, IGNORE_DOUBLE_CLICK_DEFAULT,
              N_("Prevent pause/play from triggering on double click*"),
              N_("Useful if you don't want the video to pause/play when double "
              "clicking to fullscreen. Note that enabling this will delay "
              "pause/play action by the double click interval, so the experience "
              "might not be as snappy as with this option disabled."
              "\n\n*Forces the use of the custom double click interval."), false)
    set_section(N_("Mouse button assignment"), NULL)
    _add_bool(DISABLE_FS_TOGGLE_CFG, DISABLE_FS_TOGGLE_DEFAULT,
              N_("Disable fullscreen toggle on double click"),
              N_("The video will no longer fullscreen if you double click on it."), false)
    _add_integer(FS_TOGGLE_MOUSE_BUTTON_CFG, FS_TOGGLE_MOUSE_BUTTON_DEFAULT,
                 N_("Assign fullscreen toggle to"),
                 N_("Assigns fullscreen toggle to a mouse button."), false)
    change_integer_list(mouse_button_values_index, mouse_button_names)
    _add_bool(DISABLE_CONTEXT_MENU_TOGGLE_CFG, DISABLE_CONTEXT_MENU_TOGGLE_DEFAULT,
              N_("Disable context menu toggle on right click"),
              N_("The context menu will no longer pop up if you right click on the video. "
              "Useful if you want to pause/play or full screen on right click."), false)
    _add_integer(CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_CFG, CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_DEFAULT,
                 N_("Assign context menu toggle to"),
                 N_("Assigns context menu toggle to a mouse button."), false)
    change_integer_list(mouse_button_values_index, mouse_button_names)
        add_submodule()
        set_capability("interface", 0)
#if LIBVLC_VERSION_MAJOR <= 3
        set_category(CAT_INTERFACE)
#endif
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
    if (!title) {
        vlc_player_Unlock(player);
        return false;
    }
    if (title->flags & VLC_PLAYER_TITLE_MENU || title->flags & VLC_PLAYER_TITLE_INTERACTIVE) {
        vlc_player_Unlock(player);
        return true;
    }

    vlc_player_Unlock(player);
    return false;
#endif
}

static int is_interlaced() {
    if (!p_intf) {
        return -1;
    }

#if 2 <= LIBVLC_VERSION_MAJOR && LIBVLC_VERSION_MAJOR <= 3
    playlist_t* p_playlist = pl_Get(p_intf);

    input_thread_t* p_input = playlist_CurrentInput(p_playlist);
    if (!p_input) {
        return -1;
    }

    vout_thread_t** pp_vout;
    size_t i_vout;
    if (input_Control(p_input, INPUT_GET_VOUTS, &pp_vout, &i_vout) != VLC_SUCCESS) {
        vlc_object_release(p_input);
        return -1;
    }

    int found = 0;
    for (size_t i = 0; i < i_vout; i ++) {
        int64_t deinterlace = var_GetInteger(pp_vout[i], "deinterlace");
        bool deinterlace_needed = var_GetBool(pp_vout[i], "deinterlace-needed");
        msg_Dbg(p_intf, "vout %zu/%zu: deinterlace=%" PRId64 ", deinterlace-needed=%d",
                i, i_vout, deinterlace, deinterlace_needed);
        if (deinterlace_needed) {
            found = 1;
        }
        vlc_object_release((vlc_object_t *)pp_vout[i]);
    }
    vlc_object_release(p_input);
    free(pp_vout);

    return found;
#elif LIBVLC_VERSION_MAJOR >= 4
    vlc_player_t* player = vlc_playlist_GetPlayer(vlc_intf_GetMainPlaylist(p_intf));
    vlc_player_Lock(player);

    vout_thread_t** pp_vout;
    size_t i_vout;
    pp_vout = vlc_player_vout_HoldAll(player, &i_vout);
    if (!pp_vout) {
        vlc_player_Unlock(player);
        return -1;
    }
    int found = 0;
    for (size_t i = 0; i < i_vout; i ++) {
        int64_t deinterlace = var_GetInteger(pp_vout[i], "deinterlace");
        bool deinterlace_needed = var_GetBool(pp_vout[i], "deinterlace-needed");
        msg_Dbg(p_intf, "vout %zu/%zu: deinterlace=%" PRId64 ", deinterlace-needed=%d",
                i, i_vout, deinterlace, deinterlace_needed);
        if (deinterlace_needed) {
            found = 1;
        }
        vout_Release(pp_vout[i]);
    }
    vlc_player_Unlock(player);
    free(pp_vout);

    return found;
#endif

    return -1;
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
        msg_Dbg(p_intf, "in a menu, not pausing/playing");
        return;
    }

    msg_Dbg(p_intf, "pausing/playing");

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
    if (!atomic_load(&timer_scheduled)) {
        return;
    }

    filter_t *p_filter = (filter_t *) data;
    msg_Dbg(p_filter, "delayed click: timer expired");
    if (var_InheritBool(p_filter, IGNORE_DOUBLE_CLICK_CFG)) {
        pause_play();
    }

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
    *p_mouse_out = *p_mouse_new;

    // we don't want to process anything if no mouse button was clicked
    if (p_mouse_new->i_pressed == 0 && !p_mouse_new->b_double_click) {
        return VLC_SUCCESS;
    }

#define MSG "old: i_pressed=%d, b_double_click=%d; " \
            "new: i_pressed=%d, b_double_click=%d", \
            p_mouse_old->i_pressed, p_mouse_old->b_double_click, \
            p_mouse_new->i_pressed, p_mouse_new->b_double_click
    if (p_mouse_new->b_double_click) {
        msg_Dbg(p_filter, "DOUBLE CLICK " MSG);
    } else if (p_mouse_old->i_pressed == 0 && p_mouse_new->i_pressed != 0) {
        msg_Dbg(p_filter, "PRESSED " MSG);
    } else if (p_mouse_old->i_pressed != 0 && p_mouse_new->i_pressed == 0) {
        msg_Dbg(p_filter, "RELEASED " MSG);
    } else {
        msg_Dbg(p_filter, "UNKNOWN " MSG);
    }
#undef MSG

    // get mouse button from settings. updates if user changes the setting
    const int mouse_button = cfg_get_mouse_button((vlc_object_t *)p_filter, MOUSE_BUTTON_CFG, MOUSE_BUTTON_DEFAULT);
    msg_Dbg(p_filter, "mouse_button=%d", mouse_button);

    // manually control double click to fullscreen if directly requested or if the ignore double click option is set
    if ((var_InheritBool(p_filter, ENABLE_DOUBLE_CLICK_DELAY_CFG) || var_InheritBool(p_filter, IGNORE_DOUBLE_CLICK_CFG)) &&
            mouse_button == MOUSE_BUTTON_LEFT) {
        msg_Dbg(p_filter, "manually controlling double click");
        p_mouse_out->b_double_click = 0;
    }

    // react only on the configured mouse button click
    if (vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, mouse_button)
#if LIBVLC_VERSION_MAJOR <= 3
            // treat the double click as the left mouse button click
            || (p_mouse_new->b_double_click && mouse_button == MOUSE_BUTTON_LEFT)
#endif
            ) {
        // pause/play on every click unless told not to
        if (!var_InheritBool(p_filter, IGNORE_DOUBLE_CLICK_CFG)) {
            pause_play();
        }
        // do the double click logic
        if ((var_InheritBool(p_filter, IGNORE_DOUBLE_CLICK_CFG) || var_InheritBool(p_filter, ENABLE_DOUBLE_CLICK_DELAY_CFG)) &&
                mouse_button == MOUSE_BUTTON_LEFT && timer_initialized) {
            if (atomic_load(&timer_scheduled)) {
                // it's a double click -- cancel the scheduled timer
                atomic_store(&timer_scheduled, false);
                vlc_timer_schedule(timer, false, 0, 0);
                // and set fullscreen
                p_mouse_out->b_double_click = 1;
                msg_Dbg(p_filter, "delayed click: a double click! cancelling the timer");
            } else {
                // it might be a single click -- schedule a timer
                atomic_store(&timer_scheduled, true);
                int64_t delay = var_InheritInteger(p_filter, DOUBLE_CLICK_DELAY_CFG);
                vlc_timer_schedule(timer, false, delay*1000, 0);
                msg_Dbg(p_filter, "delayed click: got a click, could it be a double-click? starting a timer for %" PRId64 "ms", delay);
            }
        }
    }

    // prevent fullscreen from toggling on double click
    if (var_InheritBool(p_filter, DISABLE_FS_TOGGLE_CFG) && p_mouse_new->b_double_click) {
        p_mouse_out->b_double_click = 0;
    }

    // toggle fullscreen on specified mouse click
    const int fs_mouse_button = cfg_get_mouse_button((vlc_object_t *)p_filter, FS_TOGGLE_MOUSE_BUTTON_CFG,
                                                     FS_TOGGLE_MOUSE_BUTTON_DEFAULT);
    if (fs_mouse_button != -1 && vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, fs_mouse_button)) {
#if LIBVLC_VERSION_MAJOR >= 4
        vlc_mouse_SetPressed(p_mouse_out, MOUSE_BUTTON_LEFT);
#endif
        p_mouse_out->b_double_click = 1;
    }

    // prevent the context menu from toggling on right click
    if (var_InheritBool(p_filter, DISABLE_CONTEXT_MENU_TOGGLE_CFG) &&
            vlc_mouse_IsPressed(p_mouse_new, MOUSE_BUTTON_RIGHT)) {
        p_mouse_out->i_pressed = 0;
    }

    // toggle context menu on specified mouse click
    const int context_menu_mouse_button =
            cfg_get_mouse_button((vlc_object_t *)p_filter,CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_CFG,
                                 CONTEXT_MENU_TOGGLE_MOUSE_BUTTON_DEFAULT);
    if (context_menu_mouse_button != -1 &&
            vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, context_menu_mouse_button)) {
        vlc_mouse_SetPressed(p_mouse_out, MOUSE_BUTTON_RIGHT);
    }

    msg_Dbg(p_filter, "out: i_pressed=%d, b_double_click=%d", p_mouse_out->i_pressed, p_mouse_out->b_double_click);

    return VLC_SUCCESS;
}

static picture_t *filter(filter_t *p_filter, picture_t *p_pic_in)
{
    UNUSED(p_filter);

    // don't alter picture
    return p_pic_in;
}

#if LIBVLC_VERSION_MAJOR >= 4
// VLC 4.0 changed mouse callback's signature, so we adapt it to the VLC2/VLC3
// callback in order to use the same function for all versions
static int _mouse(filter_t *p_filter, vlc_mouse_t *p_mouse_new_out, const vlc_mouse_t *p_mouse_old)
{
    vlc_mouse_t p_mouse_new = *p_mouse_new_out;
    return mouse(p_filter, p_mouse_new_out, p_mouse_old, &p_mouse_new);
}

static const struct vlc_filter_operations filter_ops =
{
    .filter_video = filter,
    .video_mouse = _mouse,
    .close = CloseFilter,
};
#endif

static void print_version(vlc_object_t *p_obj)
{
    msg_Dbg(p_obj, "v" VERSION_STRING ", " VERSION_COPYRIGHT ", " VERSION_LICENSE);
    msg_Dbg(p_obj, VERSION_HOMEPAGE);
}

static int OpenFilter(vlc_object_t *p_this)
{
    filter_t *p_filter = (filter_t *) p_this;

    print_version(p_this);
    msg_Dbg(p_filter, "filter sub-plugin opened");
    if (!p_intf) {
        msg_Err(p_filter, "interface sub-plugin is not initialized. "
                "Did you tick \"Pause/Play video on mouse click\" checkbox in "
                "Preferences -> All -> Interface -> Control interfaces? "
                "Don't forget to restart VLC afterwards");
        return VLC_EGENERIC;
    }
    video_format_Print(p_this, "pause_click FORMAT IN:", &p_filter->fmt_in.video);
    video_format_Print(p_this, "pause_click FORMAT OUT:", &p_filter->fmt_out.video);
    msg_Dbg(p_filter, "b_allow_fmt_out_change=%d", p_filter->b_allow_fmt_out_change);
    int interlaced = is_interlaced();
    msg_Dbg(p_filter, "is_interlaced()=%d", interlaced);

    if (p_filter->fmt_out.video.i_chroma != p_filter->fmt_in.video.i_chroma) {
        msg_Err(p_filter, "this filter doesn't do video conversion");
        return VLC_EGENERIC;
    }

#if LIBVLC_VERSION_MAJOR >= 4
    p_filter->ops = &filter_ops;
#else
    p_filter->pf_video_filter = filter;
    p_filter->pf_video_mouse = mouse;
#endif

    if (vlc_timer_create(&timer, &timer_callback, p_filter)) {
        msg_Err(p_filter, "failed to create a timer");
        return VLC_EGENERIC;
    }
    timer_initialized = true;
    atomic_store(&timer_scheduled, false);

    return VLC_SUCCESS;
}

static void CloseFilter(vlc_object_t *p_this)
{
    msg_Dbg(p_this, "filter sub-plugin closed");

    if (timer_initialized) {
        vlc_timer_destroy(timer);
        timer_initialized = false;
        atomic_store(&timer_scheduled, false);
    }
}

static int OpenInterface(vlc_object_t *p_this)
{
    p_intf = (intf_thread_t*) p_this;

    print_version(p_this);
    msg_Dbg(p_intf, "interface sub-plugin opened");

    return VLC_SUCCESS;
}

static void CloseInterface(vlc_object_t *p_this)
{
    msg_Dbg(p_this, "interface sub-plugin closed");

    p_intf = NULL;
}
