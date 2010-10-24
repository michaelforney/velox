/* velox: velox/window.c
 *
 * Copyright (c) 2009 Michael Forney <mforney@mforney.org>
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <xcb/xcb_atom.h>

#include "velox.h"
#include "window.h"
#include "debug.h"

bool window_has_protocol(xcb_window_t window, xcb_atom_t protocol)
{
    xcb_get_property_cookie_t protocols_cookie;
    xcb_get_property_reply_t * protocols_reply;
    xcb_atom_t * protocols;
    uint16_t protocols_length, index;

    protocols_cookie = xcb_get_property(c, false, window, WM_PROTOCOLS, XCB_ATOM_ATOM, 0, UINT_MAX);
    protocols_reply = xcb_get_property_reply(c, protocols_cookie, NULL);

    protocols = (xcb_atom_t *) xcb_get_property_value(protocols_reply);
    protocols_length = xcb_get_property_value_length(protocols_reply);

    for (index = 0; index < protocols_length; index++)
    {
        if (protocols[index] == protocol) return true;
    }

    return false;
}

void window_set_geometry(struct velox_window * window, struct velox_area * area)
{
    window->x = area->x;
    window->y = area->y;
    window->width = area->width - 2 * window->border_width;
    window->height = area->height - 2 * window->border_width;
}

void update_name_class(struct velox_window * window)
{
    xcb_get_property_cookie_t wm_name_cookie, wm_class_cookie;
    xcb_get_property_reply_t * wm_name_reply, * wm_class_reply;

    wm_name_cookie = xcb_get_property(c, false, window->window_id,
        XCB_ATOM_WM_NAME, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);
    wm_class_cookie = xcb_get_property(c, false, window->window_id,
        XCB_ATOM_WM_CLASS, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);

    wm_name_reply = xcb_get_property_reply(c, wm_name_cookie, NULL);
    wm_class_reply = xcb_get_property_reply(c, wm_class_cookie, NULL);

    DEBUG_PRINT("wm_name: %s\n", xcb_get_property_value(wm_name_reply))
    DEBUG_PRINT("wm_class: %s\n", xcb_get_property_value(wm_class_reply))

    window->name = strndup(xcb_get_property_value(wm_name_reply),
        xcb_get_property_value_length(wm_name_reply));
    window->class = strndup(xcb_get_property_value(wm_class_reply),
        xcb_get_property_value_length(wm_class_reply));

    free(wm_name_reply);
    free(wm_class_reply);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

