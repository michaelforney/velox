/* mwm: mwm.c
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

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>

#include "window.h"

/* X specific variables */
xcb_connection_t * c;
xcb_screen_t * screen;
xcb_window_t root;

/* X atoms */
enum wm_atom_t
{
    WM_PROTOCOLS,
    WM_DELETE_WINDOW,
    WM_STATE
};
enum net_atom_t
{
    NET_SUPPORTED,
    NET_WM_NAME
};

xcb_atom_t wm_atoms[3];
xcb_atom_t net_atoms[2];

/* MWM variables */
bool running = true;

void setup()
{
    xcb_screen_iterator_t screen_iterator;
    xcb_intern_atom_cookie_t * wm_atom_cookies, * net_atom_cookies;
    uint32_t mask;
    uint32_t values[1];

    window_initialize();

    c = xcb_connect(NULL, NULL);

    screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(c));

    screen = screen_iterator.data;
    root = screen->root;

    /* Setup atoms */
    wm_atom_cookies = (xcb_intern_atom_cookie_t *) malloc(3 * sizeof(xcb_intern_atom_cookie_t));
    net_atom_cookies = (xcb_intern_atom_cookie_t *) malloc(2 * sizeof(xcb_intern_atom_cookie_t));

    wm_atom_cookies[WM_PROTOCOLS] = xcb_intern_atom(c, false, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
    wm_atom_cookies[WM_DELETE_WINDOW] = xcb_intern_atom(c, false, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    wm_atom_cookies[WM_STATE] = xcb_intern_atom(c, false, strlen("WM_STATE"), "WM_STATE");

    net_atom_cookies[NET_SUPPORTED] = xcb_intern_atom(c, false, strlen("_NET_SUPPORTED"), "_NET_SUPPORTED");
    net_atom_cookies[NET_WM_NAME] = xcb_intern_atom(c, false, strlen("_NET_WM_NAME"), "_NET_WM_NAME");

    wm_atoms[WM_PROTOCOLS] = xcb_intern_atom_reply(c, wm_atom_cookies[WM_PROTOCOLS], NULL)->atom;
    wm_atoms[WM_DELETE_WINDOW] = xcb_intern_atom_reply(c, wm_atom_cookies[WM_DELETE_WINDOW], NULL)->atom;
    wm_atoms[WM_STATE] = xcb_intern_atom_reply(c, wm_atom_cookies[WM_STATE], NULL)->atom;

    net_atoms[NET_SUPPORTED] = xcb_intern_atom_reply(c, net_atom_cookies[NET_SUPPORTED], NULL)->atom;
    net_atoms[NET_WM_NAME] = xcb_intern_atom_reply(c, net_atom_cookies[NET_WM_NAME], NULL)->atom;

    free(wm_atom_cookies);
    free(net_atom_cookies);

    mask = XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_PROPERTY_CHANGE;

    xcb_configure_window(c, root, mask, values);
}

void manage(xcb_window_t window_id, xcb_get_window_attributes_reply_t * window_attributes)
{
    struct mwm_window * window;
    struct mwm_window * transient;
    xcb_get_property_cookie_t transient_for_cookie;
    xcb_get_property_reply_t * transient_for_reply;

    transient_for_cookie = xcb_get_property(c, false, window_id, WM_TRANSIENT_FOR, WINDOW, 0, 1);

    window = (struct mwm_window *) malloc(sizeof(struct mwm_window));

    window->window_id = window_id;
    
    transient_for_reply = xcb_get_property_reply(c, transient_for_cookie, NULL);
    if (transient_for_reply)
    {
        xcb_window_t transient_id;

        transient_id = *((xcb_window_t *) xcb_get_property_value);
        transient = window_lookup(transient_id);
    }
    if (transient)
    {
        window->tags = transient->tags;
    }
    else
    {
    }
}

void unmanage(struct mwm_window * window)
{
}

void manage_existing_windows()
{
    xcb_query_tree_cookie_t query_cookie;
    xcb_query_tree_reply_t * query;
    xcb_window_t * children;
    int child, child_count;
    xcb_get_window_attributes_cookie_t * window_attribute_cookies;
    xcb_get_property_cookie_t * property_cookies;

    query_cookie = xcb_query_tree(c, root);
    query = xcb_query_tree_reply(c, query_cookie, NULL);
    children = xcb_query_tree_children(query);

    window_attribute_cookies = (xcb_get_window_attributes_cookie_t *) malloc(child_count * sizeof(xcb_get_window_attributes_cookie_t));
    property_cookies = (xcb_get_property_cookie_t *) malloc(child_count * sizeof(xcb_get_property_cookie_t));

    for (child = 0; child < child_count; child++)
    {
        window_attribute_cookies[child] = xcb_get_window_attributes(c, children[child]);
        property_cookies[child] = xcb_get_property(c, false, children[child], WM_TRANSIENT_FOR, WINDOW, 0, 1);
    }

    for (child = 0; child < child_count; child++)
    {
        xcb_get_window_attributes_reply_t * window_attributes = xcb_get_window_attributes_reply(c, window_attribute_cookies[child], NULL);
        xcb_get_property_reply_t * transient_for = xcb_get_property_reply(c, property_cookies[child], NULL);

        if (window_attributes->override_redirect || xcb_get_property_value(transient_for))
        {
            continue;
        }
        if (window_attributes->map_state == XCB_MAP_STATE_VIEWABLE)
        {
            manage(children[child], window_attributes);
        }
    }
    for (child = 0; child < child_count; child++)
    {
        xcb_get_window_attributes_reply_t * window_attributes = xcb_get_window_attributes_reply(c, window_attribute_cookies[child], NULL);
        xcb_get_property_reply_t * transient_for = xcb_get_property_reply(c, property_cookies[child], NULL);

        if (xcb_get_property_value(transient_for) && window_attributes->map_state == XCB_MAP_STATE_VIEWABLE)
        {
            manage(children[child], window_attributes);
        }
    }
}

/* X event handlers */
void button_press(xcb_button_press_event_t * event)
{
    printf("button_press\n");
}

void configure_request(xcb_configure_request_event_t * event)
{
    printf("configure_request\n");
}

void configure_notify(xcb_configure_notify_event_t * event)
{
    printf("configure_notify\n");
}

void destroy_notify(xcb_destroy_notify_event_t * event)
{
    struct mwm_window * window;

    printf("destroy_notify\n");

    
}

void enter_notify(xcb_enter_notify_event_t * event)
{
    printf("enter_notify\n");
}

void expose(xcb_expose_event_t * event)
{
    printf("expose\n");

    if (event->count == 0)
    {
        bar_draw();
    }
}

void focus_in(xcb_focus_in_event_t * event)
{
    printf("focus_in\n");
}

void key_press(xcb_key_press_event_t * event)
{
    printf("key_press\n");
}

void mapping_notify(xcb_mapping_notify_event_t * event)
{
    printf("mapping_notify\n");
}

void map_request(xcb_map_request_event_t * event)
{
    printf("map_request\n");
}

void property_notify(xcb_property_notify_event_t * event)
{
    printf("property_notify\n");
}

void unmap_notify(xcb_unmap_notify_event_t * event)
{
    printf("unmap_notify\n");
}

void run()
{
    xcb_generic_event_t * event;
    while (running && (event = xcb_wait_for_event(c)))
    {
        switch (event->response_type & ~0x80)
        {
            case XCB_BUTTON_PRESS:
                button_press((xcb_button_press_event_t *)event);
            case XCB_CONFIGURE_REQUEST:
                configure_request((xcb_configure_request_event_t *)event);
            case XCB_CONFIGURE_NOTIFY:
                configure_notify((xcb_configure_notify_event_t *)event);
            case XCB_DESTROY_NOTIFY:
                destroy_notify((xcb_destroy_notify_event_t *)event);
            case XCB_ENTER_NOTIFY:
                enter_notify((xcb_enter_notify_event_t *)event);
            case XCB_EXPOSE:
                expose((xcb_expose_event_t *)event);
            case XCB_FOCUS_IN:
                focus_in((xcb_focus_in_event_t *)event);
            case XCB_KEY_PRESS:
                key_press((xcb_key_press_event_t *)event);
            case XCB_MAPPING_NOTIFY:
                mapping_notify((xcb_mapping_notify_event_t *) event);
            case XCB_MAP_REQUEST:
                map_request((xcb_map_request_event_t *) event);
            case XCB_PROPERTY_NOTIFY:
                property_notify((xcb_property_notify_event_t *) event);
            case XCB_UNMAP_NOTIFY:
                unmap_notify((xcb_unmap_notify_event_t *) event);

            default:
                break;
        }

        free(event);
    }
}

void cleanup()
{
    xcb_disconnect(c);
}

int main(int argc, char ** argv)
{
    setup();
    manage_existing_windows();
    run();
    cleanup();

    return 0;
}

