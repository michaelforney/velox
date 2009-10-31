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
#include "hook.h"

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

static const uint8_t wm_atoms_size = 3;
static const uint8_t net_atoms_size = 2;

xcb_atom_t wm_atoms[wm_atoms_size];
xcb_atom_t net_atoms[net_atoms_size];

/* X cursors */
enum cursor_id_t
{
    POINTER_ID = 68,
    RESIZE_ID = 120,
    MOVE_ID = 52
};
enum cursor_type_t
{
    POINTER,
    RESIZE,
    MOVE
};

static const uint8_t cursor_type_size = 3;

xcb_cursor_t cursors[cursor_type_size];

/* MWM variables */
bool running = true;

void setup()
{
    xcb_screen_iterator_t screen_iterator;
    xcb_font_t cursor_font;
    xcb_intern_atom_cookie_t * wm_atom_cookies, * net_atom_cookies;
    uint32_t mask;
    uint32_t values[2];

    window_initialize();

    c = xcb_connect(NULL, NULL);

    screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(c));

    screen = screen_iterator.data;
    root = screen->root;

    /* Setup atoms */
    wm_atom_cookies = (xcb_intern_atom_cookie_t *) malloc(wm_atoms_size * sizeof(xcb_intern_atom_cookie_t));
    net_atom_cookies = (xcb_intern_atom_cookie_t *) malloc(net_atoms_size * sizeof(xcb_intern_atom_cookie_t));

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

    /* Setup cursors */
    cursor_font = xcb_generate_id(c);
    xcb_open_font(c, cursor_font, strlen("cursor"), "cursor");

    cursors[POINTER] = xcb_generate_id(c);
    cursors[RESIZE] = xcb_generate_id(c);
    cursors[MOVE] = xcb_generate_id(c);

    xcb_create_glyph_cursor(c, cursors[POINTER], cursor_font, cursor_font, POINTER_ID, POINTER_ID + 1, 0, 0, 0, 0, 0, 0);
    xcb_create_glyph_cursor(c, cursors[RESIZE], cursor_font, cursor_font, RESIZE_ID, RESIZE_ID + 1, 0, 0, 0, 0, 0, 0);
    xcb_create_glyph_cursor(c, cursors[MOVE], cursor_font, cursor_font, MOVE_ID, MOVE_ID + 1, 0, 0, 0, 0, 0, 0);

    xcb_change_property(c, XCB_PROP_MODE_REPLACE, root, net_atoms[NET_SUPPORTED], ATOM, 32, net_atoms_size, net_atoms);

    mask = XCB_CW_EVENT_MASK | XCB_CW_CURSOR;
    values[0] = XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_PROPERTY_CHANGE;
    values[1] = cursors[POINTER];

    xcb_configure_window(c, root, mask, values);
}

void configure_window(struct mwm_window * window)
{
    xcb_configure_notify_event_t event;

    event.response_type = XCB_CONFIGURE_NOTIFY;
    event.event = window->window_id;
    event.window = window->window_id;
    event.above_sibling = 0;
    event.x = window->x;
    event.y = window->y;
    event.width = window->width;
    event.height = window->height;
    event.border_width = window->border_width;
    event.override_redirect = false;

    xcb_send_event(c, false, window->window_id, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *) &event);
}

void manage(xcb_window_t window_id)
{
    printf("manage(%i)\n", window_id);

    struct mwm_window * window = NULL;
    struct mwm_window * transient = NULL;
    xcb_get_property_cookie_t transient_for_cookie;
    xcb_get_property_reply_t * transient_for_reply = NULL;
    xcb_get_geometry_cookie_t geometry_cookie;
    xcb_get_geometry_reply_t * geometry = NULL;
    xcb_window_t transient_id = 0;
    uint32_t mask;
    uint32_t values[1];

    transient_for_cookie = xcb_get_property(c, false, window_id, WM_TRANSIENT_FOR, WINDOW, 0, 1);
    geometry_cookie = xcb_get_geometry(c, window_id);

    window = (struct mwm_window *) malloc(sizeof(struct mwm_window));

    window->window_id = window_id;
    
    transient_for_reply = xcb_get_property_reply(c, transient_for_cookie, NULL);
    transient_id = *((xcb_window_t *) xcb_get_property_value(transient_for_reply));

    free(transient_for_reply);

    if (transient_id)
    {
        transient = window_lookup(transient_id);
        window->tags = transient->tags;
    }
    else
    {
        manage_hooks_apply(window);
    }

    /* Geometry */
    geometry = xcb_get_geometry_reply(c, geometry_cookie, NULL);

    window->x = geometry->x;
    window->y = geometry->y;
    window->width = geometry->width;
    window->height = geometry->height;

    /* Events */
    mask = XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_FOCUS_CHANGE |
                XCB_EVENT_MASK_PROPERTY_CHANGE |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_configure_window(c, window_id, mask, values);
}

void unmanage(struct mwm_window * window)
{
}

void manage_existing_windows()
{
    printf("manage_existing_windows()\n");

    xcb_query_tree_cookie_t query_cookie;
    xcb_query_tree_reply_t * query;
    xcb_window_t * children;
    uint16_t child, child_count;
    xcb_get_window_attributes_cookie_t * window_attributes_cookies;
    xcb_get_window_attributes_reply_t ** window_attributes_replies;
    xcb_get_property_cookie_t * property_cookies;
    xcb_get_property_reply_t ** property_replies;

    query_cookie = xcb_query_tree(c, root);
    query = xcb_query_tree_reply(c, query_cookie, NULL);
    children = xcb_query_tree_children(query);
    child_count = xcb_query_tree_children_length(query);

    window_attributes_cookies = (xcb_get_window_attributes_cookie_t *) malloc(child_count * sizeof(xcb_get_window_attributes_cookie_t));
    window_attributes_replies = (xcb_get_window_attributes_reply_t **) malloc(child_count * sizeof(xcb_get_window_attributes_reply_t *));
    property_cookies = (xcb_get_property_cookie_t *) malloc(child_count * sizeof(xcb_get_property_cookie_t));
    property_replies = (xcb_get_property_reply_t **) malloc(child_count * sizeof(xcb_get_property_reply_t *));

    for (child = 0; child < child_count; child++)
    {
        window_attributes_cookies[child] = xcb_get_window_attributes(c, children[child]);
        property_cookies[child] = xcb_get_property(c, false, children[child], WM_TRANSIENT_FOR, WINDOW, 0, 1);
    }
    for (child = 0; child < child_count; child++)
    {
        window_attributes_replies[child] = xcb_get_window_attributes_reply(c, window_attributes_cookies[child], NULL);
        property_replies[child] = xcb_get_property_reply(c, property_cookies[child], NULL);

        if (window_attributes_replies[child]->override_redirect || *(xcb_window_t *) xcb_get_property_value(property_replies[child]))
        {
            printf("override_redirect or transient\n");
            continue;
        }
        if (window_attributes_replies[child]->map_state == XCB_MAP_STATE_VIEWABLE)
        {
            manage(children[child]);
        }
    }
    for (child = 0; child < child_count; child++)
    {
        if (*(xcb_window_t *) xcb_get_property_value(property_replies[child]) && window_attributes_replies[child]->map_state == XCB_MAP_STATE_VIEWABLE)
        {
            manage(children[child]);
        }

        free(window_attributes_replies[child]);
        free(property_replies[child]);
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
                button_press((xcb_button_press_event_t *) event);
            case XCB_CONFIGURE_REQUEST:
                configure_request((xcb_configure_request_event_t *) event);
            case XCB_CONFIGURE_NOTIFY:
                configure_notify((xcb_configure_notify_event_t *) event);
            case XCB_DESTROY_NOTIFY:
                destroy_notify((xcb_destroy_notify_event_t *) event);
            case XCB_ENTER_NOTIFY:
                enter_notify((xcb_enter_notify_event_t *) event);
            case XCB_EXPOSE:
                expose((xcb_expose_event_t *) event);
            case XCB_FOCUS_IN:
                focus_in((xcb_focus_in_event_t *) event);
            case XCB_KEY_PRESS:
                key_press((xcb_key_press_event_t *) event);
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
    /* X cursors */
    xcb_free_cursor(c, cursors[POINTER]);
    xcb_free_cursor(c, cursors[RESIZE]);
    xcb_free_cursor(c, cursors[MOVE]);

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

