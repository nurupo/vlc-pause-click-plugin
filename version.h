/*****************************************************************************
 * version.h : Version and software information
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
#define VERSION_MAJOR 2
#define VERSION_MINOR 2
#define VERSION_PATCH 0

// release vs in-development version
#define VERSION_IS_RELEASE 0

#define VERSION_AUTHOR    "Maxim Biro (nurupo)"
#define VERSION_COPYRIGHT "(C) 2014-2023 " VERSION_AUTHOR
#define VERSION_LICENSE   "LGPL-2.1-or-later"
#define VERSION_HOMEPAGE  "https://github.com/nurupo/vlc-pause-click-plugin"

#define XSTR(s) STR(s)
#define STR(s) #s

#if VERSION_IS_RELEASE
    #define VERSION_STRING \
                XSTR(VERSION_MAJOR) "." XSTR(VERSION_MINOR) "." XSTR(VERSION_PATCH)
#else
    #define VERSION_STRING \
                XSTR(VERSION_MAJOR) "." XSTR(VERSION_MINOR) "." XSTR(VERSION_PATCH) "-dev"
#endif
