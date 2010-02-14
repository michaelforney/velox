/* velox: velox/window.c
 *
 * Copyright (c) 2009 Michael Forney <michael@obberon.com>
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

#include "velox.h"
#include "window.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <xcb/xcb_atom.h>

struct velox_window * window_list_lookup(struct velox_list * list, xcb_window_t window_id)
{
    struct velox_list * iterator;
    struct velox_window * window;

    for (iterator = list; iterator != NULL; iterator = iterator->next)
    {
        window = (struct velox_window *) iterator->data;

        if (window->window_id == window_id)
        {
            return window;
        }
    }

    return NULL;
}

struct velox_list * window_list_delete(struct velox_list * list, xcb_window_t window_id)
{
    struct velox_list * iterator;
    struct velox_window * window;

    window = (struct velox_window *) list->data;

    if (window->window_id == window_id)
    {
        return velox_list_remove_first(list);
    }

    for (iterator = list; iterator != NULL; iterator = iterator->next)
    {
        window = (struct velox_window *) iterator->data;

        if (window->window_id == window_id)
        {
            velox_list_remove_first(iterator);
        }
    }

    return list;
}

struct velox_window * window_loop_lookup(struct velox_loop * loop, xcb_window_t window_id)
{
    struct velox_loop * iterator;

    if (loop == NULL)
    {
        return NULL;
    }
    else
    {
        struct velox_window * window;

        iterator = loop;
        do
        {
            window = (struct velox_window *) iterator->data;

            if (window->window_id == window_id)
            {
                return window;
            }

            iterator = iterator->next;
        } while (iterator != loop);
    }

    return NULL;
}

struct velox_loop * window_loop_locate(struct velox_loop * loop, xcb_window_t window_id)
{
    struct velox_loop * iterator;

    if (loop == NULL)
    {
        return NULL;
    }
    else
    {
        struct velox_window * window;

        iterator = loop;
        do
        {
            if (((struct velox_window *) iterator->data)->window_id == window_id)
            {
                return iterator;
            }

            iterator = iterator->next;
        } while (iterator != loop);
    }

    return NULL;
}

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

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

