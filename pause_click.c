/*****************************************************************************
 * pause_click.c : A filter that allows to pause/play a video by a mouse click
 *****************************************************************************
 * Copyright (C) 2014-2017 Maxim Biro
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

#include <vlc_atomic.h>
#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_mouse.h>
#include <vlc_playlist.h>
#include <vlc_plugin.h>
#include <vlc_threads.h>

#include <vlc/libvlc_version.h>

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

#define TO_CHAR(num) ( 'A' + (char)(num) )
#define FROM_CHAR(c) ( (int)( (c) - 'A' ) )

#define MOUSE_BUTTON_LIST \
    SELECT_COLUMN(N_("Left Button"),    MOUSE_BUTTON_LEFT,        0), \
    SELECT_COLUMN(N_("Middle Button"),  MOUSE_BUTTON_CENTER,      1), \
    SELECT_COLUMN(N_("Right Button"),   MOUSE_BUTTON_RIGHT,       2), \
    SELECT_COLUMN(N_("Scroll Up"),      MOUSE_BUTTON_WHEEL_UP,    3), \
    SELECT_COLUMN(N_("Scroll Down"),    MOUSE_BUTTON_WHEEL_DOWN,  4), \
    SELECT_COLUMN(N_("Scroll Left"),    MOUSE_BUTTON_WHEEL_LEFT,  5), \
    SELECT_COLUMN(N_("Scroll Right"),   MOUSE_BUTTON_WHEEL_RIGHT, 6)

#define SELECT_COLUMN(NAME, VALUE, INDEX) NAME
static const char *const mouse_button_names[] = { MOUSE_BUTTON_LIST };
#undef SELECT_COLUMN

#define SELECT_COLUMN(NAME, VALUE, INDEX) TO_CHAR(VALUE)
static const char mouse_button_values_string[] = { MOUSE_BUTTON_LIST , 0 };
#undef SELECT_COLUMN

#define SELECT_COLUMN(NAME, VALUE, INDEX) mouse_button_values_string + INDEX
static const char *const mouse_button_values[] = { MOUSE_BUTTON_LIST };
#undef SELECT_COLUMN

#define SETTINGS_PREFIX "pause-click-"

#define MOUSE_BUTTON_SETTING SETTINGS_PREFIX "mouse-button-setting"
#define MOUSE_BUTTON_DEFAULT mouse_button_values_string // MOUSE_BUTTON_LEFT

#define DOUBLE_CLICK_ENABLED_SETTING SETTINGS_PREFIX "double-click-setting"
#define DOUBLE_CLICK_ENABLED_DEFAULT false

#define DOUBLE_CLICK_DELAY_SETTING SETTINGS_PREFIX "double-click-delay-setting"
#define DOUBLE_CLICK_DELAY_DEFAULT 300


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
    add_string(MOUSE_BUTTON_SETTING, MOUSE_BUTTON_DEFAULT, N_("Mouse button"),
               N_("Defines the mouse button that will pause/play the video."), false)
    change_string_list(mouse_button_values, mouse_button_names)
    add_bool(DOUBLE_CLICK_ENABLED_SETTING, DOUBLE_CLICK_ENABLED_DEFAULT,
             N_("Ignore double clicks"), N_("Useful if you don't want the video to "
             "pause when double clicking to fullscreen. Note that enabling this "
             "will delay pause/play action by the double click interval, so the "
             "experience might not be as snappy as with this option disabled."), false)
    add_integer_with_range(DOUBLE_CLICK_DELAY_SETTING, DOUBLE_CLICK_DELAY_DEFAULT,
                           20, 5000, N_("Double click interval (milliseconds)"),
                           N_("Two clicks made during this time interval will be "
                           "treated as a double click and will be ignored."), false)
        add_submodule()
        set_capability("interface", 0)
        set_category(CAT_INTERFACE)
        set_subcategory(SUBCAT_INTERFACE_CONTROL)
        set_callbacks(OpenInterface, CloseInterface)
vlc_module_end()


static void pause_play(void)
{
    if (p_intf == NULL) {
        return;
    }

    playlist_t* p_playlist = pl_Get(p_intf);
    playlist_Control(p_playlist,
                     (playlist_Status(p_playlist) == PLAYLIST_RUNNING ? PLAYLIST_PAUSE : PLAYLIST_PLAY), 0);
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

static int mouse(filter_t *p_filter, vlc_mouse_t *p_mouse_out, const vlc_mouse_t *p_mouse_old, const vlc_mouse_t *p_mouse_new)
{
    UNUSED(p_mouse_out);

    // we don't want to process anything if no mouse button was clicked
    if (p_mouse_new->i_pressed == 0 && !p_mouse_new->b_double_click) {
        return VLC_EGENERIC;
    }

    // get mouse button from settings. updates if user changes the setting
    char *mouse_button_value = var_InheritString(p_filter, MOUSE_BUTTON_SETTING);
    if (mouse_button_value == NULL) {
        return VLC_EGENERIC;
    }
    int mouse_button = FROM_CHAR(mouse_button_value[0]);
    free(mouse_button_value);

    if (vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, mouse_button) ||
            // on some systems (e.g. Linux) b_double_click is not set for a double-click, so we track any click and
            // decide if it was a double click on our own. This provides the most uniform cross-platform behaviour.
            (p_mouse_new->b_double_click && mouse_button == MOUSE_BUTTON_LEFT)) {
        // if ignoring double click
        if (var_InheritBool(p_filter, DOUBLE_CLICK_ENABLED_SETTING) && timer_initialized) {
            if (atomic_load(&timer_scheduled)) {
                // it's a double click -- cancel the scheduled pause/play, if any
                atomic_store(&timer_scheduled, false);
                vlc_timer_schedule(timer, false, 0, 0);
            } else {
                // it might be a single click -- schedule pause/play call
                atomic_store(&timer_scheduled, true);
                vlc_timer_schedule(timer, false, var_InheritInteger(p_filter, DOUBLE_CLICK_DELAY_SETTING)*1000, 0);
            }
        } else {
            pause_play();
        }
    }

    // don't propagate any mouse change
    return VLC_EGENERIC;
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
