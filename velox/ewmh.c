/* velox: velox/ewmh.c
 *
 * Copyright (c) 2010 Michael Forney <mforney@mforney.org>
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
#include <string.h>
#include <unistd.h>
#include <xcb/xcb_ewmh.h>

#include "velox.h"
#include "work_area.h"
#include "hook.h"
#include "debug.h"
#include "vector.h"
#include "event_handler.h"

#include "ewmh-private.h"

xcb_ewmh_connection_t * ewmh;

struct velox_vector client_list;

static void __attribute__((constructor)) initialize_client_list()
{
    vector_initialize(&client_list, sizeof(xcb_window_t), 32);
}

static void __attribute__((destructor)) free_client_list()
{
    vector_free(&client_list);
}

static void update_client_list()
{
    DEBUG_ENTER

    if (client_list.size == 0) xcb_ewmh_set_client_list(ewmh, 0, 0, NULL);
    else
    {
        xcb_ewmh_set_client_list(ewmh, 0, client_list.size, client_list.data);
    }
}

static void supporting_wm()
{
    xcb_window_t child_id;
    pid_t wm_pid;

    wm_pid = getpid();

    child_id = xcb_generate_id(c);
    xcb_create_window(
        c,
        XCB_COPY_FROM_PARENT,
        child_id,
        screen->root,
        -1, -1, 1, 1,
        0,
        XCB_COPY_FROM_PARENT,
        screen->root_visual,
        0, NULL
    );

    /* Set _NET_SUPPORTING_WM_CHECK on the root window */
    xcb_ewmh_set_supporting_wm_check(ewmh, 0, child_id);

    /* Set _NET_SUPPORTING_WM_CHECK on the newly created window */
    xcb_change_property(
        c,
        XCB_PROP_MODE_REPLACE,
        screen->root,
        ewmh->_NET_SUPPORTING_WM_CHECK,
        XCB_ATOM_WINDOW,
        32, 1, &child_id
    );

    xcb_ewmh_set_wm_name(ewmh, child_id, strlen(wm_name), wm_name);
    xcb_ewmh_set_wm_pid(ewmh, child_id, wm_pid);
}

static void avoid_struts(const struct velox_area * screen_area,
    struct velox_area * work_area)
{
    xcb_query_tree_cookie_t query_cookie;
    xcb_query_tree_reply_t * query_reply;
    xcb_window_t * children;
    xcb_get_property_cookie_t * strut_cookies;
    xcb_ewmh_wm_strut_partial_t strut_partial;
    xcb_ewmh_get_extents_reply_t strut;
    uint16_t index, child_count, strut_count = 0;
    uint32_t x0, y0, x1, y1;

    query_cookie = xcb_query_tree(c, screen->root);

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
    free(query_reply);
}

static void add_client_hook(union velox_argument argument)
{
    struct velox_window * window = (struct velox_window *) argument.pointer;

    DEBUG_ENTER

    vector_add_value(&client_list, window->window_id);

    update_client_list();
}

static void remove_client_hook(union velox_argument argument)
{
    struct velox_window * window = (struct velox_window *) argument.pointer;
    xcb_window_t * window_id;

    DEBUG_ENTER

    DEBUG_PRINT("window_id: 0x%x\n", window->window_id)

    vector_for_each(&client_list, window_id)
    {
        if (*window_id == window->window_id)
        {
            vector_remove_at(&client_list, vector_position(&client_list, window_id));
            break;
        }
    }

    update_client_list();
}

static void update_clients_hook(union velox_argument argument)
{
    struct velox_tag * tag = (struct velox_tag *) argument.pointer;
    struct velox_window_entry * entry;

    DEBUG_ENTER

    vector_clear(&client_list);

    list_for_each_entry(entry, &tag->tiled.windows, head)
    {
        vector_add_value(&client_list, entry->window->window_id);
    }

    update_client_list();
}

static void desktop_geometry_hook(union velox_argument argument)
{
    xcb_ewmh_set_desktop_geometry(ewmh, 0, screen_area.width, screen_area.height);
}

static void focus_hook(union velox_argument argument)
{
    const xcb_window_t * window_id = (const xcb_window_t *) argument.pointer;

    if (*window_id == screen->root)
    {
        xcb_ewmh_set_active_window(ewmh, 0, XCB_WINDOW_NONE);
    }
    else
    {
        xcb_ewmh_set_active_window(ewmh, 0, *window_id);
    }
}

static void handle_client_message(xcb_client_message_event_t * event)
{
    DEBUG_ENTER

    if (event->type == ewmh->_NET_ACTIVE_WINDOW)
    {
        struct list_head * old_focus;

        DEBUG_PRINT("window: 0x%x\n", event->data.data32[0])

        old_focus = tag->tiled.focus;

        /* Assume the client message is valid */
        for (tag->tiled.focus = list_actual_next(tag->tiled.focus, &tag->tiled.windows);
            tag->tiled.focus != old_focus;
            tag->tiled.focus = list_actual_next(tag->tiled.focus, &tag->tiled.windows))
        {
            if (list_entry(
                    tag->tiled.focus, struct velox_window_entry, head
                )->window->window_id == event->window)
            {
                break;
            }
        }

        focus(list_entry(tag->tiled.focus, struct velox_window_entry, head)->window->window_id);
    }
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
            ewmh->_NET_CLIENT_LIST,
            /* ewmh->_NET_CLIENT_LIST_STACKING, */
            /* ewmh->_NET_NUMBER_OF_DESKTOPS, */
            ewmh->_NET_DESKTOP_GEOMETRY,
            ewmh->_NET_DESKTOP_VIEWPORT,
            /* ewmh->_NET_CURRENT_DESKTOP, */
            /* ewmh->_NET_DESKTOP_NAMES, */
            ewmh->_NET_ACTIVE_WINDOW,
            /* ewmh->_NET_WORKAREA, */
            ewmh->_NET_SUPPORTING_WM_CHECK,
            /* ewmh->_NET_VIRTUAL_ROOTS, */
            /* ewmh->_NET_DESKTOP_LAYOUT, */
            /* ewmh->_NET_SHOWING_DESKTOP, */
            /* ewmh->_NET_CLOSE_WINDOW, */
            /* ewmh->_NET_MOVERESIZE_WINDOW, */
            /* ewmh->_NET_WM_MOVERESIZE, */
            /* ewmh->_NET_RESTACK_WINDOW, */
            /* ewmh->_NET_REQUEST_FRAME_EXTENTS, */
            /* ewmh->_NET_WM_NAME, */
            /* ewmh->_NET_WM_VISIBLE_NAME, */
            /* ewmh->_NET_WM_ICON_NAME, */
            /* ewmh->_NET_WM_VISIBLE_ICON_NAME, */
            /* ewmh->_NET_WM_DESKTOP, */
            /* ewmh->_NET_WM_WINDOW_TYPE, */
            /* ewmh->_NET_WM_STATE, */
            /* ewmh->_NET_WM_ALLOWED_ACTIONS, */
            ewmh->_NET_WM_STRUT,
            ewmh->_NET_WM_STRUT_PARTIAL,
            /* ewmh->_NET_WM_ICON_GEOMETRY, */
            /* ewmh->_NET_WM_ICON, */
            /* ewmh->_NET_WM_PID, */
            /* ewmh->_NET_WM_HANDLED_ICONS, */
            /* ewmh->_NET_WM_USER_TIME, */
            /* ewmh->_NET_WM_USER_TIME_WINDOW, */
            /* ewmh->_NET_FRAME_EXTENTS, */
            /* ewmh->_NET_WM_PING, */
            /* ewmh->_NET_WM_SYNC_REQUEST, */
            /* ewmh->_NET_WM_SYNC_REQUEST_COUNTER, */
            /* ewmh->_NET_WM_FULLSCREEN_MONITORS, */
            /* ewmh->_NET_WM_FULL_PLACEMENT, */
        };

        xcb_ewmh_set_supported(ewmh, 0, sizeof(supported) / sizeof(xcb_atom_t), supported);
    }

    supporting_wm();

    /* Trivial properties */
    xcb_ewmh_set_desktop_geometry(ewmh, 0, screen_area.width, screen_area.height);
    xcb_ewmh_set_desktop_viewport(ewmh, 0, 0, 0);

    add_client_message_event_handler(&handle_client_message);
    add_work_area_modifier(avoid_struts);

    /* Client list updates */
    add_hook(add_client_hook, VELOX_HOOK_MANAGE_POST);
    add_hook(remove_client_hook, VELOX_HOOK_UNMANAGE);
    add_hook(update_clients_hook, VELOX_HOOK_TAG_CHANGED);

    add_hook(desktop_geometry_hook, VELOX_HOOK_ROOT_RESIZED);

    add_hook(focus_hook, VELOX_HOOK_FOCUS_CHANGED);
}

void cleanup_ewmh()
{
    free(ewmh);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

