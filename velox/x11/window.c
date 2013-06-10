/* velox: velox/x11/window.c
 *
 * Copyright (c) 2009, 2013 Michael Forney <mforney@mforney.org>
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

#include <xcb/xcb.h>

#include "x11.h"

#include "velox.h"
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

void update_name_class(struct velox_window * window)
{
    uint32_t wm_name_length;
    uint32_t wm_class_length;

    xcb_get_property_cookie_t wm_name_cookie, wm_class_cookie;
    xcb_get_property_reply_t * wm_name_reply, * wm_class_reply;

    wm_name_cookie = xcb_get_property(c, false, window->window_id,
        XCB_ATOM_WM_NAME, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);
    wm_class_cookie = xcb_get_property(c, false, window->window_id,
        XCB_ATOM_WM_CLASS, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);

    wm_name_reply = xcb_get_property_reply(c, wm_name_cookie, NULL);
    wm_class_reply = xcb_get_property_reply(c, wm_class_cookie, NULL);

    wm_name_length = MIN(sizeof(window->name) - 1,
        xcb_get_property_value_length(wm_name_reply));
    wm_class_length = MIN(sizeof(window->class) - 1,
        xcb_get_property_value_length(wm_class_reply));

    memcpy(window->name, xcb_get_property_value(wm_name_reply), wm_name_length);
    memcpy(window->class, xcb_get_property_value(wm_class_reply), wm_class_length);
    window->name[wm_name_length] = '\0';
    window->class[wm_class_length] = '\0';

    DEBUG_PRINT("wm_name: %s\n", window->name)
    DEBUG_PRINT("wm_class: %s\n", window->class)

    free(wm_name_reply);
    free(wm_class_reply);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

