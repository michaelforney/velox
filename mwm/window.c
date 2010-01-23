/* mwm: mwm/window.c
 *
 * Copyright (c) 2009 Michael Forney <michael@obberon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mwm.h"
#include "window.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <xcb/xcb_atom.h>

struct mwm_window * window_list_lookup(struct mwm_list * list, xcb_window_t window_id)
{
    struct mwm_list * iterator;
    struct mwm_window * window;

    for (iterator = list; iterator != NULL; iterator = iterator->next)
    {
        window = (struct mwm_window *) iterator->data;

        if (window->window_id == window_id)
        {
            return window;
        }
    }

    return NULL;
}

struct mwm_list * window_list_delete(struct mwm_list * list, xcb_window_t window_id)
{
    struct mwm_list * iterator;
    struct mwm_window * window;

    window = (struct mwm_window *) list->data;

    if (window->window_id == window_id)
    {
        return mwm_list_remove_first(list);
    }

    for (iterator = list; iterator != NULL; iterator = iterator->next)
    {
        window = (struct mwm_window *) iterator->data;

        if (window->window_id == window_id)
        {
            mwm_list_remove_first(iterator);
        }
    }

    return list;
}

struct mwm_window * window_loop_lookup(struct mwm_loop * loop, xcb_window_t window_id)
{
    struct mwm_loop * iterator;

    if (loop == NULL)
    {
        return NULL;
    }
    else
    {
        struct mwm_window * window;

        iterator = loop;
        do
        {
            window = (struct mwm_window *) iterator->data;

            if (window->window_id == window_id)
            {
                return window;
            }

            iterator = iterator->next;
        } while (iterator != loop);
    }

    return NULL;
}

struct mwm_loop * window_loop_locate(struct mwm_loop * loop, xcb_window_t window_id)
{
    struct mwm_loop * iterator;

    if (loop == NULL)
    {
        return NULL;
    }
    else
    {
        struct mwm_window * window;

        iterator = loop;
        do
        {
            if (((struct mwm_window *) iterator->data)->window_id == window_id)
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

    protocols_cookie = xcb_get_property(c, false, window, WM_PROTOCOLS, ATOM, 0, UINT_MAX);
    protocols_reply = xcb_get_property_reply(c, protocols_cookie, NULL);

    protocols = (xcb_atom_t *) xcb_get_property_value(protocols_reply);
    protocols_length = xcb_get_property_value_length(protocols_reply);

    for (index = 0; index < protocols_length; index++)
    {
        if (protocols[index] == protocol)
        {
            printf("found protocol: %i\n", index);
            printf("protocol: %i\n", protocol);
            return true;
        }
    }

    return false;
}

