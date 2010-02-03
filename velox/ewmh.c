/* velox: velox/ewmh.c
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of velox.
 *
 * velox is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * velox is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with velox.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb_ewmh.h>

#include "velox.h"

#include "ewmh-private.h"

xcb_ewmh_connection_t * ewmh;

void setup_ewmh()
{
    xcb_intern_atom_cookie_t * ewmh_cookies;

    ewmh = (xcb_ewmh_connection_t *) malloc(sizeof(xcb_ewmh_connection_t));

    ewmh_cookies = xcb_ewmh_init_atoms(c, ewmh);
    xcb_ewmh_init_atoms_replies(ewmh, ewmh_cookies, NULL);

    {
        xcb_atom_t supported[] = {
            ewmh->_NET_SUPPORTED,
        };

        xcb_ewmh_set_supported(ewmh, 0, sizeof(supported) / sizeof(xcb_atom_t), supported);
    }
}

void cleanup_ewmh()
{
}

