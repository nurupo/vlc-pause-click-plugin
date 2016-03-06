/*****************************************************************************
 * pause_click.c : A filter that allows to pause/play a video by a mouse click
 *****************************************************************************
 * Copyright (C) 2014 Maxim Biro
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define N_(str) (str)
#endif

#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_interface.h>
#include <vlc_mouse.h>
#include <vlc_playlist.h>
#include <vlc_plugin.h>


#define UNUSED(x) (void)(x)

#define TO_CHAR(num) ( 'A' + (char)(num) )
#define FROM_CHAR(c) ( (int)( (c) - 'A' ) )

#define MOUSE_BUTTON_LIST \
    SELECT_COLUMN("Left Button",    MOUSE_BUTTON_LEFT,        0), \
    SELECT_COLUMN("Middle Button",  MOUSE_BUTTON_CENTER,      1), \
    SELECT_COLUMN("Right Button",   MOUSE_BUTTON_RIGHT,       2), \
    SELECT_COLUMN("Scroll Up",      MOUSE_BUTTON_WHEEL_UP,    3), \
    SELECT_COLUMN("Scroll Down",    MOUSE_BUTTON_WHEEL_DOWN,  4), \
    SELECT_COLUMN("Scroll Left",    MOUSE_BUTTON_WHEEL_LEFT,  5), \
    SELECT_COLUMN("Scroll Right",   MOUSE_BUTTON_WHEEL_RIGHT, 6)

#define SELECT_COLUMN(NAME, VALUE, INDEX) NAME
static const char *const mouse_button_names[] = { MOUSE_BUTTON_LIST };
#undef SELECT_COLUMN

#define SELECT_COLUMN(NAME, VALUE, INDEX) TO_CHAR(VALUE)
static const char mouse_button_values_string[] = { MOUSE_BUTTON_LIST , 0 };
#undef SELECT_COLUMN

#define SELECT_COLUMN(NAME, VALUE, INDEX) mouse_button_values_string + INDEX
static const char *const mouse_button_values[] = { MOUSE_BUTTON_LIST };
#undef SELECT_COLUMN

#define MOUSE_BUTTON_SETTING "mouse-button-setting"
#define MOUSE_BUTTON_DEFAULT mouse_button_values_string // MOUSE_BUTTON_LEFT


int OpenFilter(vlc_object_t *);
int OpenInterface(vlc_object_t *);

intf_thread_t *p_intf = NULL;


vlc_module_begin()
    set_description("Pause/Play video on mouse click")
    set_shortname("Pause click")
    set_capability("video filter2", 0)
    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VFILTER)
    set_callbacks(OpenFilter, NULL)
    add_string(MOUSE_BUTTON_SETTING, MOUSE_BUTTON_DEFAULT, "Mouse Button",
               "Defines the mouse button that will pause/play the video", false)
    change_string_list(mouse_button_values, mouse_button_names)
        add_submodule()
        set_capability("interface", 0)
        set_category(CAT_INTERFACE)
        set_subcategory(SUBCAT_INTERFACE_CONTROL)
        set_callbacks(OpenInterface, NULL)
vlc_module_end()


int mouse(filter_t *p_filter, vlc_mouse_t *p_mouse_out, const vlc_mouse_t *p_mouse_old, const vlc_mouse_t *p_mouse_new)
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

    if (p_intf != NULL &&
            (vlc_mouse_HasPressed(p_mouse_old, p_mouse_new, mouse_button) || p_mouse_new->b_double_click)) {
        playlist_t* p_playlist = pl_Get(p_intf);
        playlist_Control(p_playlist,
                         (playlist_Status(p_playlist) == PLAYLIST_RUNNING ? PLAYLIST_PAUSE : PLAYLIST_PLAY), 0);
    }

    // don't propagate any mouse change
    return VLC_EGENERIC;
}

picture_t *filter(filter_t *p_filter, picture_t *p_pic_in)
{
    UNUSED(p_filter);

    // don't alter picture
    return p_pic_in;
}

int OpenFilter(vlc_object_t *p_this)
{
    filter_t *p_filter = (filter_t *)p_this;

    p_filter->pf_video_filter = filter;
    p_filter->pf_video_mouse = mouse;

    return VLC_SUCCESS;
}

int OpenInterface(vlc_object_t *p_this)
{
    p_intf = (intf_thread_t*) p_this;

    return VLC_SUCCESS;
}
