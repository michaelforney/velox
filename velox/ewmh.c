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
#include "work_area.h"

#include "ewmh-private.h"

xcb_ewmh_connection_t * ewmh;

void avoid_struts(const struct velox_area * screen_area, struct velox_area * work_area)
{
    xcb_query_tree_cookie_t query_cookie;
    xcb_query_tree_reply_t * query_reply;
    xcb_window_t * children;
    xcb_get_property_cookie_t * strut_cookies;
    xcb_ewmh_wm_strut_partial_t strut_partial;
    xcb_ewmh_get_extents_reply_t strut;
    uint16_t index, child_count, strut_count = 0;
    uint32_t x0, y0, x1, y1;

    query_cookie = xcb_query_tree(c, root);

    x0 = 0;
    y0 = 0;
    x1 = screen_area->width;
    y1 = screen_area->height;

    query_reply = xcb_query_tree_reply(c, query_cookie, NULL);
    children = xcb_query_tree_children(query_reply);
    child_count = xcb_query_tree_children_length(query_reply);

    strut_cookies = (xcb_get_property_cookie_t *) malloc(child_count * sizeof(xcb_get_property_cookie_t));

    for (index = 0; index < child_count; ++index)
    {
        strut_cookies[index] = xcb_ewmh_get_wm_strut_partial(ewmh, children[index]);
    }
    for (index = 0; index < child_count; ++index)
    {
        /* If the client supports _NET_WM_STRUT_PARTIAL */
        if (xcb_ewmh_get_wm_strut_partial_reply(ewmh,
            strut_cookies[index], &strut_partial, NULL))
        {
            x0 = MAX(x0, strut_partial.left);
            y0 = MAX(y0, strut_partial.top);
            x1 = MIN(x1, screen_area->width - strut_partial.right);
            y1 = MIN(y1, screen_area->height - strut_partial.bottom);
        }
        /* Otherwise ask for _NET_WM_STRUT */
        else
        {
            strut_cookies[strut_count++] = xcb_ewmh_get_wm_strut(ewmh, children[index]);
        }
    }
    for (index = 0; index < strut_count; ++index)
    {
        /* If the client is a strut */
        if (xcb_ewmh_get_wm_strut_reply(ewmh, strut_cookies[index], &strut, NULL))
        {
            x0 = MAX(x0, strut.left);
            y0 = MAX(y0, strut.top);
            x1 = MIN(x1, screen_area->width - strut.right);
            y1 = MIN(y1, screen_area->height - strut.bottom);
        }
    }

    work_area->x = x0;
    work_area->y = y0;
    work_area->width = x1 - x0;
    work_area->height = y1 - y0;

    free(strut_cookies);
}

void setup_ewmh()
{
    xcb_intern_atom_cookie_t * ewmh_cookies;

    ewmh = (xcb_ewmh_connection_t *) malloc(sizeof(xcb_ewmh_connection_t));

    ewmh_cookies = xcb_ewmh_init_atoms(c, ewmh);
    xcb_ewmh_init_atoms_replies(ewmh, ewmh_cookies, NULL);

    {
        xcb_atom_t supported[] = {
            ewmh->_NET_SUPPORTED,
            ewmh->_NET_WM_STRUT,
            ewmh->_NET_WM_STRUT_PARTIAL
        };

        xcb_ewmh_set_supported(ewmh, 0, sizeof(supported) / sizeof(xcb_atom_t), supported);
    }

    add_work_area_modifier(avoid_struts);
}

void cleanup_ewmh()
{
    free(ewmh);
}

