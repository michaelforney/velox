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
#include <xcb/xcb_icccm.h>

#include "window.h"
#include "hook.h"

/* X specific variables */
xcb_connection_t * c;
xcb_screen_t * screen;
xcb_window_t root;

uint16_t screen_width = 0;
uint16_t screen_height = 0;

/* X atoms */
enum
{
    WM_PROTOCOLS,
    WM_DELETE_WINDOW,
    WM_STATE
};
enum
{
    NET_SUPPORTED,
    NET_WM_NAME
};

static const uint8_t wm_atoms_size = 3;
static const uint8_t net_atoms_size = 2;

xcb_atom_t wm_atoms[wm_atoms_size];
xcb_atom_t net_atoms[net_atoms_size];

/* X cursors */
enum
{
    POINTER_ID = 68,
    RESIZE_ID = 120,
    MOVE_ID = 52
};
enum
{
    POINTER,
    RESIZE,
    MOVE
};

static const uint8_t cursor_type_size = 3;

xcb_cursor_t cursors[cursor_type_size];

/* MWM variables */
bool running = true;
uint64_t current_tags = 0;
struct mwm_window_stack * stack = NULL;

void setup()
{
    xcb_screen_iterator_t screen_iterator;
    xcb_font_t cursor_font;
    xcb_intern_atom_cookie_t * wm_atom_cookies, * net_atom_cookies;
    uint32_t mask;
    uint32_t values[2];

    c = xcb_connect(NULL, NULL);

    screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(c));

    screen = screen_iterator.data;
    root = screen->root;
    screen_width = screen->width_in_pixels;
    screen_height = screen->height_in_pixels;

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
    values[1] = cursors[RESIZE];

    xcb_change_window_attributes(c, root, mask, values);
}

/**
 * Sends a synthetic configure request to the window
 *
 * @param window The window to send the request to
 */
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

void arrange()
{
    printf("arrange()\n");

    struct mwm_window_stack * current_element = stack;

    for (; current_element != NULL; current_element = current_element->next)
    {
        struct mwm_window * window = current_element->window;
        uint16_t mask;
        uint32_t values[4];

        printf("window: %i\n", window);

        window->x = 0;
        window->y = 0;
        window->width = 960;
        window->height = 600;

        printf("window: %i\n", window);

        mask = XCB_CONFIG_WINDOW_X |
               XCB_CONFIG_WINDOW_Y |
               XCB_CONFIG_WINDOW_WIDTH |
               XCB_CONFIG_WINDOW_HEIGHT;
        values[0] = window->x;
        values[1] = window->y;
        values[2] = window->width;
        values[3] = window->height;

        printf("window: %i\n", window);

        xcb_configure_window(c, window->window_id, mask, values);

        configure_window(window);
    }

    printf("end loop\n");
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
    uint32_t property_values[2];

    transient_for_cookie = xcb_get_property(c, false, window_id, WM_TRANSIENT_FOR, WINDOW, 0, 1);
    geometry_cookie = xcb_get_geometry(c, window_id);

    window = (struct mwm_window *) malloc(sizeof(struct mwm_window));

    window->window_id = window_id;
    
    transient_for_reply = xcb_get_property_reply(c, transient_for_cookie, NULL);
    transient_id = *((xcb_window_t *) xcb_get_property_value(transient_for_reply));

    if (transient_for_reply->type == WINDOW && transient_id)
    {
        printf("transient_id: %i\n", transient_id);
        transient = window_stack_lookup(stack, transient_id);
        window->tags = transient->tags;
    }
    else
    {
        manage_hooks_apply(window);
    }

    free(transient_for_reply);

    /* Geometry */
    geometry = xcb_get_geometry_reply(c, geometry_cookie, NULL);

    printf("x: %i, y: %i, width: %i, height: %i\n", geometry->x, geometry->y, geometry->width, geometry->height);

    window->x = geometry->x;
    window->y = geometry->y;
    window->width = geometry->width;
    window->height = geometry->height;

    configure_window(window);

    /* Events */
    mask = XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_FOCUS_CHANGE |
                XCB_EVENT_MASK_PROPERTY_CHANGE |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_change_window_attributes(c, window_id, mask, values);

    stack = window_stack_insert(stack, window);

    xcb_map_window(c, window->window_id);

    property_values[0] = XCB_WM_STATE_NORMAL;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id, wm_atoms[WM_STATE], WM_HINTS, 32, 2, property_values);

    arrange();
}

void unmanage(struct mwm_window * window)
{
    uint32_t property_values[2];

    stack = window_stack_delete(stack, window->window_id);

    property_values[0] = XCB_WM_STATE_WITHDRAWN;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id, wm_atoms[WM_STATE], WM_HINTS, 32, 2, property_values);

    free(window);
}

void focus(struct mwm_window * window)
{
    // TODO: Implement
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
    
    struct mwm_window * window = NULL;

    window = window_stack_lookup(stack, event->window);

    if (window)
    {
        printf("configure_request for already managed window... ignoring\n");

        /* Case 3 of the ICCCM 4.1.5 */
        if (event->value_mask & (XCB_CONFIG_WINDOW_BORDER_WIDTH | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT))
        {
            window->border_width = event->border_width;
            // TODO: Make sure this is right
        }
        /* Case 1 of the ICCCM 4.1.5 */
        else
        {
            configure_window(window);
        }
    }
    /* Case 2 of the ICCCM 4.1.5 */
    else
    {
        uint16_t mask = 0;
        uint32_t values[7];
        uint8_t field = 0;
        
        if (event->value_mask & XCB_CONFIG_WINDOW_X)
        {
            values[field++] = event->x;
            mask |= XCB_CONFIG_WINDOW_X;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_Y)
        {
            values[field++] = event->y;
            mask |= XCB_CONFIG_WINDOW_Y;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_WIDTH)
        {
            values[field++] = event->width;
            mask |= XCB_CONFIG_WINDOW_WIDTH;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
        {
            values[field++] = event->height;
            mask |= XCB_CONFIG_WINDOW_HEIGHT;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_SIBLING)
        {
            values[field++] = event->sibling;
            mask |= XCB_CONFIG_WINDOW_SIBLING;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)
        {
            values[field++] = event->stack_mode;
            mask |= XCB_CONFIG_WINDOW_STACK_MODE;
        }

        xcb_configure_window(c, event->window, mask, values);
    }
}

void configure_notify(xcb_configure_notify_event_t * event)
{
    printf("configure_notify\n");

    if (event->window == root)
    {
        printf("window is root\n");
        screen_width = event->width;
        screen_height = event->height;
    }
}

void destroy_notify(xcb_destroy_notify_event_t * event)
{
    struct mwm_window * window;

    printf("destroy_notify\n");

    window = window_stack_lookup(stack, event->window);
    if (window)
    {
        unmanage(window);
    }
}

void enter_notify(xcb_enter_notify_event_t * event)
{
    struct mwm_window * window;

    printf("enter_notify\n");

    window = window_stack_lookup(stack, event->event);
    if (window)
    {
        focus(window);
    }
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
    
    // TODO: Prevent focus stealing?
}

void key_press(xcb_key_press_event_t * event)
{
    printf("key_press\n");
    
    // TODO: Handle key shortcuts
}

void mapping_notify(xcb_mapping_notify_event_t * event)
{
    printf("mapping_notify\n");

    if (event->response_type == XCB_MAPPING_KEYBOARD)
    {
        /* TODO: Deal with keyboard map changes
         * This probably needs xcb-keysyms */
    }
}

void map_request(xcb_map_request_event_t * event)
{
    printf("map_request\n");
    
    struct mwm_window * maybe_window;
    xcb_get_window_attributes_cookie_t window_attributes_cookie;
    xcb_get_window_attributes_reply_t * window_attributes;

    window_attributes_cookie = xcb_get_window_attributes(c, event->window);

    printf("window_id: %i\n", event->window);

    maybe_window = window_stack_lookup(stack, event->window);

    printf("maybe_window: %i\n", maybe_window);

    window_attributes = xcb_get_window_attributes_reply(c, window_attributes_cookie, NULL);

    printf("window_attributes: %i\n", window_attributes);

    if (!maybe_window && window_attributes && !window_attributes->override_redirect)
    {
        manage(event->window);
    }
}

void property_notify(xcb_property_notify_event_t * event)
{
    printf("property_notify\n");

    xcb_get_atom_name_cookie_t atom_name_cookie;
    xcb_get_atom_name_reply_t * atom_name;

    atom_name_cookie = xcb_get_atom_name(c, event->atom);
    atom_name = xcb_get_atom_name_reply(c, atom_name_cookie, NULL);

    if (atom_name)
    {
        //printf("atom: %s\n", xcb_get_atom_name_name(atom_name));
    }
}

void unmap_notify(xcb_unmap_notify_event_t * event)
{
    printf("unmap_notify: %i\n", event->window);

    struct mwm_window * window;

    window = window_stack_lookup(stack, event->window);

    if (window)
    {
        unmanage(window);
    }
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

