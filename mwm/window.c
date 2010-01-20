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

struct mwm_window * window_list_lookup(struct mwm_window_list * list, xcb_window_t window_id)
{
    for (; list != NULL && list->window->window_id != window_id; list = list->next);

    if (list == NULL)
    {
        return NULL;
    }

    return list->window;
}

struct mwm_window_list * window_list_delete(struct mwm_window_list * list, xcb_window_t window_id)
{
    struct mwm_window_list * previous_element = NULL;
    struct mwm_window_list * current_element = NULL;

    if (list == NULL)
    {
        return NULL;
    }
    else if (list->window->window_id == window_id)
    {
        struct mwm_window_list * new_list = list->next;

        free(list);
        return new_list;
    }

    for (previous_element = list, current_element = list->next; current_element->window->window_id != window_id && current_element != NULL; previous_element = current_element, current_element = current_element->next);

    if (current_element != NULL)
    {
        previous_element->next = current_element->next;
        free(current_element);
    }

    return list;
}

struct mwm_window_list * window_list_move_to_front(struct mwm_window_list * list, xcb_window_t window_id)
{
    struct mwm_window_list * previous_element = NULL;
    struct mwm_window_list * current_element = NULL;

    if (list == NULL)
    {
        return NULL;
    }
    else if (list->window->window_id == window_id)
    {
        return list;
    }

    for (previous_element = list, current_element = list->next; current_element->window->window_id != window_id && current_element != NULL; previous_element = current_element, current_element = current_element->next);

    if (current_element != NULL)
    {
        previous_element->next = current_element->next;
        current_element->next = list;

        return current_element;
    }

    return list;
}

struct mwm_window_list * window_list_insert(struct mwm_window_list * list, struct mwm_window * window)
{
    struct mwm_window_list * new_list;

    new_list = (struct mwm_window_list *) malloc(sizeof(struct mwm_window_list));

    new_list->window = window;
    new_list->next = list;

    return new_list;
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

