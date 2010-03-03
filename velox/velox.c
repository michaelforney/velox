/* velox: velox/velox.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
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
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_aux.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include "velox.h"
#include "window.h"
#include "tag.h"
#include "hook.h"
#include "keybinding.h"
#include "config_file.h"
#include "debug.h"
#include "list.h"

#include "module-private.h"
#include "config_file-private.h"
#include "keybinding-private.h"
#include "hook-private.h"
#include "layout-private.h"
#include "ewmh-private.h"
#include "event-private.h"
#include "work_area-private.h"

/* X variables */
xcb_connection_t * c;
xcb_screen_t * screen;
xcb_window_t root;

/* X atoms */
const uint16_t atom_length = 3;
xcb_atom_t WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE;

/* X cursors */
enum
{
    POINTER_ID = XC_left_ptr,
    RESIZE_ID = XC_sizing,
    MOVE_ID = XC_fleur
};
enum
{
    POINTER,
    RESIZE,
    MOVE
};

xcb_cursor_t cursors[3];

/* VELOX variables */
bool running = true;
uint64_t tag_mask = 0;
struct velox_tag * tag = NULL;
struct velox_area screen_area;
struct velox_area work_area;
uint16_t pending_unmaps = 0;
uint8_t clear_event_type = 0;

uint32_t border_pixel;
uint32_t border_focus_pixel;

uint16_t border_width = 2;

/* VELOX constants */
const char wm_name[] = "velox";
const uint16_t border_color[] = { 0x9999, 0x9999, 0x9999 };
const uint16_t border_focus_color[] = { 0x3333,  0x8888, 0x3333 };

struct velox_window_entry * lookup_window_entry(xcb_window_t window_id)
{
    struct velox_tag ** tag;
    struct velox_window_entry * window_entry;

    vector_for_each(&tags, tag)
    {
        list_for_each_entry(window_entry, &(*tag)->tiled.windows, head)
        {
            if (window_entry->window->window_id == window_id)
            {
                return window_entry;
            }
        }

        /* TODO: look through floated windows */
    }

    return NULL;
}

void check_wm_running()
{
    uint16_t mask;
    uint32_t values[1];
    xcb_void_cookie_t change_attributes_cookie;
    xcb_generic_error_t * error;

    mask = XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
    change_attributes_cookie = xcb_change_window_attributes_checked(c, root, mask, values);

    error = xcb_request_check(c, change_attributes_cookie);
    if (error)
    {
        die("Another window manager is already running");
    }
}

void setup()
{
    const xcb_setup_t * setup;
    xcb_screen_iterator_t screen_iterator;
    xcb_font_t cursor_font;
    xcb_intern_atom_cookie_t * atom_cookies;
    xcb_intern_atom_reply_t * atom_reply;
    xcb_alloc_color_cookie_t border_color_cookie;
    xcb_alloc_color_cookie_t border_focus_color_cookie;
    xcb_alloc_color_reply_t * border_color_reply;
    xcb_alloc_color_reply_t * border_focus_color_reply;
    uint32_t mask;
    uint32_t values[2];

    c = xcb_connect(NULL, NULL);

    if (xcb_connection_has_error(c)) die("Could not open display");

    setup = xcb_get_setup(c);

    screen_iterator = xcb_setup_roots_iterator(setup);

    screen = screen_iterator.data;
    root = screen->root;
    screen_area.width = screen->width_in_pixels;
    screen_area.height = screen->height_in_pixels;

    work_area = screen_area;

    check_wm_running();

    /* Allocate colors */
    border_color_cookie = xcb_alloc_color(c, screen->default_colormap,
        border_color[0], border_color[1], border_color[2]);
    border_focus_color_cookie = xcb_alloc_color(c, screen->default_colormap,
        border_focus_color[0], border_focus_color[1], border_focus_color[2]);

    /* Setup atoms */
    atom_cookies = (xcb_intern_atom_cookie_t *) malloc(atom_length * sizeof(xcb_intern_atom_cookie_t));
    atom_cookies[0] = xcb_intern_atom(c, false, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
    atom_cookies[1] = xcb_intern_atom(c, false, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    atom_cookies[2] = xcb_intern_atom(c, false, strlen("WM_STATE"), "WM_STATE");

    /* Setup cursors */
    cursor_font = xcb_generate_id(c);
    xcb_open_font(c, cursor_font, strlen("cursor"), "cursor");

    cursors[POINTER] = xcb_generate_id(c);
    cursors[RESIZE] = xcb_generate_id(c);
    cursors[MOVE] = xcb_generate_id(c);

    xcb_create_glyph_cursor(c, cursors[POINTER], cursor_font, cursor_font,
        POINTER_ID, POINTER_ID + 1, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF);
    xcb_create_glyph_cursor(c, cursors[RESIZE], cursor_font, cursor_font,
        RESIZE_ID, RESIZE_ID + 1, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF);
    xcb_create_glyph_cursor(c, cursors[MOVE], cursor_font, cursor_font,
        MOVE_ID, MOVE_ID + 1, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF);

    mask = XCB_CW_EVENT_MASK | XCB_CW_CURSOR;
    values[0] = XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_PROPERTY_CHANGE;
    values[1] = cursors[POINTER];

    xcb_change_window_attributes(c, root, mask, values);

    /* Check color allocation replies */
    border_color_reply = xcb_alloc_color_reply(c, border_color_cookie, NULL);
    border_focus_color_reply = xcb_alloc_color_reply(c, border_focus_color_cookie, NULL);

    border_pixel = border_color_reply->pixel;
    border_focus_pixel = border_focus_color_reply->pixel;

    free(border_color_reply);
    free(border_focus_color_reply);

    atom_reply = xcb_intern_atom_reply(c, atom_cookies[0], NULL);
    WM_PROTOCOLS = atom_reply->atom;
    free(atom_reply);
    atom_reply = xcb_intern_atom_reply(c, atom_cookies[1], NULL);
    WM_DELETE_WINDOW = atom_reply->atom;
    free(atom_reply);
    atom_reply = xcb_intern_atom_reply(c, atom_cookies[2], NULL);
    WM_STATE = atom_reply->atom;
    free(atom_reply);
    atom_reply = xcb_intern_atom_reply(c, atom_cookies[3], NULL);

    free(atom_cookies);

    setup_key_bindings();
    setup_hooks();

    load_config();

    setup_modules();

    setup_tags();
    setup_ewmh();

    grab_keys(setup->min_keycode, setup->max_keycode);

    assert(tags.size > 0);
    tag = tags.data[0];
}

void show_window(xcb_window_t window_id)
{
    uint32_t property_values[2];

    DEBUG_ENTER

    property_values[0] = XCB_WM_STATE_NORMAL;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window_id, WM_STATE, WM_STATE, 32, 2, property_values);

    xcb_map_window(c, window_id);
}

void hide_window(xcb_window_t window_id)
{
    uint32_t property_values[2];

    DEBUG_ENTER

    property_values[0] = XCB_WM_STATE_WITHDRAWN; // FIXME: Maybe this should be iconic?
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window_id, WM_STATE, WM_STATE, 32, 2, property_values);

    pending_unmaps++;

    xcb_unmap_window(c, window_id);
}

/**
 * Sends a synthetic configure request to the window
 *
 * @param window The window to send the request to
 */
void synthetic_configure(struct velox_window * window)
{
    xcb_configure_notify_event_t event;

    DEBUG_ENTER

    event.response_type = XCB_CONFIGURE_NOTIFY;
    event.event = window->window_id;
    event.window = window->window_id;
    event.above_sibling = XCB_NONE;
    event.x = window->x;
    event.y = window->y;
    event.width = window->width;
    event.height = window->height;
    event.border_width = window->border_width;
    event.override_redirect = false;

    xcb_send_event(c, false, window->window_id, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *) &event);
}

/**
 * Sends a synthetic unmap request to the window
 *
 * See section 4.1.4 of the ICCCM
 *
 * @param wnidow The window to send the request to
 */
void synthetic_unmap(struct velox_window * window)
{
    xcb_unmap_notify_event_t event;

    event.response_type = XCB_UNMAP_NOTIFY;
    event.event = root;
    event.window = window->window_id;
    event.from_configure = false;

    xcb_send_event(c, false, root,
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
        (const char *) &event);
}

void focus(xcb_window_t window_id)
{
    uint16_t mask = XCB_CW_BORDER_PIXEL;
    uint32_t values[1];

    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (focus_reply->focus == window_id)
    {
        free(focus_reply);
        return;
    }

    if (window_id != root)
    {
        values[0] = border_focus_pixel;
        xcb_change_window_attributes(c, window_id, mask, values);
    }

    if (focus_reply->focus != root)
    {
        values[0] = border_pixel;
        xcb_change_window_attributes(c, focus_reply->focus, mask, values);
    }

    free(focus_reply);

    xcb_set_input_focus(c, XCB_INPUT_FOCUS_POINTER_ROOT, window_id, XCB_TIME_CURRENT_TIME);
    run_hooks(&window_id, VELOX_HOOK_FOCUS_CHANGED);

    xcb_flush(c);
}

void set_tag(void * generic_index)
{
    uint8_t index = (uint8_t) generic_index;

    DEBUG_ENTER

    assert(index < tags.size);

    if (tag == tags.data[index]) return; // Nothing to do...
    else
    {
        struct velox_window_entry * window_entry;
        struct velox_loop * iterator;
        struct velox_window * window;

        /* Show the windows now visible */
        list_for_each_entry(window_entry, &tags.data[index]->tiled.windows, head)
        {
            show_window(window_entry->window->window_id);
        }

        /* Hide windows no longer visible */
        list_for_each_entry(window_entry, &tag->tiled.windows, head)
        {
            hide_window(window_entry->window->window_id);
        }

        tag = tags.data[index];

        /* TODO: check if tag has floating focus and behave accordingly */

        if (list_empty(&tag->tiled.windows))
        {
            focus(root);
        }
        else
        {
            focus(list_entry(
                    tag->tiled.focus, struct velox_window_entry, head
                )->window->window_id);
        }

        arrange();

        run_hooks(tag, VELOX_HOOK_TAG_CHANGED);
    }
}

void move_focus_to_tag(void * generic_index)
{
    uint8_t index = (uint8_t) generic_index;

    DEBUG_ENTER

    if (list_empty(&tag->tiled.windows)) return;
    else
    {
        struct list_head * next_focus;
        struct velox_window * window;

        window = list_entry(tag->tiled.focus, struct velox_window_entry, head)->window;
        window->tag = tags.data[index];

        /* Deal with special cases */
        if (list_is_singular(&tag->tiled.windows))
        {
            /* Set focus to the list head (no focus) */
            next_focus = &tag->tiled.windows;
        }
        else
        {
            next_focus = list_actual_next(tag->tiled.focus, &tag->tiled.windows);
        }

        /* Remove the focus from the old list, and add it to the new list */
        list_del(tag->tiled.focus);
        list_add(tag->tiled.focus, &tags.data[index]->tiled.windows);

        tag->tiled.focus = next_focus;

        if (list_is_singular(&tags.data[index]->tiled.windows))
        {
            /* If the tag was empty before, set its focus to the new window */
            tags.data[index]->tiled.focus = tags.data[index]->tiled.windows.next;
        }

        if (list_empty(&tag->tiled.windows))
        {
            focus(root);
        }
        else
        {
            focus(list_entry(tag->tiled.focus, struct velox_window_entry, head)->window->window_id);
        }

        hide_window(window->window_id);

        arrange();
    }

    xcb_flush(c);
}

void next_layout()
{
    DEBUG_ENTER

    tag->layout = list_actual_next(tag->layout, &tag->layouts);
    tag->state = list_entry(tag->layout, struct velox_layout_entry, head)->layout->default_state;

    arrange();
}

void previous_layout()
{
    DEBUG_ENTER

    tag->layout = list_actual_prev(tag->layout, &tag->layouts);
    tag->state = list_entry(tag->layout, struct velox_layout_entry, head)->layout->default_state;

    arrange();
}

void focus_next()
{
    DEBUG_ENTER

    if (list_empty(&tag->tiled.windows)) return;

    tag->tiled.focus = list_actual_next(tag->tiled.focus, &tag->tiled.windows);

    focus(list_entry(
            tag->tiled.focus, struct velox_window_entry, head
        )->window->window_id);
}

void focus_previous()
{
    DEBUG_ENTER

    if (list_empty(&tag->tiled.windows)) return;

    tag->tiled.focus = list_actual_prev(tag->tiled.focus, &tag->tiled.windows);

    focus(list_entry(
            tag->tiled.focus, struct velox_window_entry, head
        )->window->window_id);
}

void move_next()
{
    struct velox_window_entry * first, * second;
    struct velox_window * first_window;

    DEBUG_ENTER

    if (list_empty(&tag->tiled.windows)) return;

    first = list_entry(tag->tiled.focus, struct velox_window_entry, head);
    second = list_entry(list_actual_next(tag->tiled.focus, &tag->tiled.windows),
        struct velox_window_entry, head);

    /* Swap the two windows */
    first_window = first->window;
    first->window = second->window;
    second->window = first_window;

    tag->tiled.focus = list_actual_next(tag->tiled.focus, &tag->tiled.windows);

    arrange();
}

void move_previous()
{
    struct velox_window_entry * first, * second;
    struct velox_window * first_window;

    DEBUG_ENTER

    if (list_empty(&tag->tiled.windows)) return;

    first = list_entry(tag->tiled.focus, struct velox_window_entry, head);
    second = list_entry(list_actual_prev(tag->tiled.focus, &tag->tiled.windows),
        struct velox_window_entry, head);

    /* Swap the two windows */
    first_window = first->window;
    first->window = second->window;
    second->window = first_window;

    tag->tiled.focus = list_actual_prev(tag->tiled.focus, &tag->tiled.windows);

    arrange();
}

void kill_focused_window()
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    DEBUG_ENTER

    if (focus_reply->focus == root)
    {
        return;
    }

    if (window_has_protocol(focus_reply->focus, WM_DELETE_WINDOW))
    {
        xcb_client_message_event_t event;

        event.response_type = XCB_CLIENT_MESSAGE;
        event.format = 32;
        event.window = focus_reply->focus;
        event.type = WM_PROTOCOLS;
        event.data.data32[0] = WM_DELETE_WINDOW;
        event.data.data32[1] = XCB_CURRENT_TIME;

        xcb_send_event(c, false, focus_reply->focus, XCB_EVENT_MASK_NO_EVENT, (char *) &event);
    }
    else xcb_kill_client(c, focus_reply->focus);

    xcb_flush(c);
}

void arrange()
{
    DEBUG_ENTER

    if (list_empty(&tag->tiled.windows)) return;

    assert(!list_empty(&tag->layouts));

    calculate_work_area(&screen_area, &work_area);
    list_entry(
        tag->layout, struct velox_layout_entry, head
    )->layout->arrange(&work_area, &tag->tiled.windows, &tag->state);

    clear_event_type = XCB_ENTER_NOTIFY;
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

void manage(xcb_window_t window_id)
{
    DEBUG_ENTER

    struct velox_window * window = NULL;
    struct velox_window_entry * window_entry;
    struct velox_window_entry * transient_entry = NULL;
    xcb_get_property_cookie_t transient_for_cookie;
    xcb_get_property_reply_t * transient_for_reply = NULL;
    xcb_get_geometry_cookie_t geometry_cookie;
    xcb_get_geometry_reply_t * geometry = NULL;
    xcb_window_t transient_id = 0;
    uint32_t mask;
    uint32_t values[2];
    uint32_t property_values[2];

    transient_for_cookie = xcb_get_property(c, false, window_id,
        XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 1);
    geometry_cookie = xcb_get_geometry(c, window_id);

    window = (struct velox_window *) malloc(sizeof(struct velox_window));
    DEBUG_PRINT("allocated window: %i\n", (uint32_t) window)
    window->window_id = window_id;

    window_entry = (struct velox_window_entry *) malloc(sizeof(struct velox_window_entry));
    window_entry->window = window;

    transient_for_reply = xcb_get_property_reply(c, transient_for_cookie, NULL);
    transient_id = *((xcb_window_t *) xcb_get_property_value(transient_for_reply));

    if (transient_for_reply->type == XCB_ATOM_WINDOW && transient_id)
    {
        DEBUG_PRINT("transient_id: %i\n", transient_id)
        transient_entry = lookup_window_entry(transient_id);

        window->floating = true;
    }
    else
    {
        window->floating = false;
    }

    if (transient_entry != NULL)
    {
        window->tag = transient_entry->window->tag;
    }
    else
    {
        window->tag = tag;
    }

    free(transient_for_reply);

    /* Geometry */
    geometry = xcb_get_geometry_reply(c, geometry_cookie, NULL);

    DEBUG_PRINT("x: %i, y: %i, width: %i, height: %i\n",
        geometry->x, geometry->y, geometry->width, geometry->height)

    window->x = geometry->x;
    window->y = geometry->y;
    window->width = geometry->width;
    window->height = geometry->height;
    window->border_width = border_width;

    free(geometry);

    update_name_class(window);

    run_hooks(window, VELOX_HOOK_MANAGE_PRE);

    mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
    values[0] = window->border_width;

    xcb_configure_window(c, window->window_id, mask, values);

    /* Events and border color */
    mask = XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = border_pixel;
    values[1] = XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_PROPERTY_CHANGE |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_change_window_attributes(c, window_id, mask, values);

    synthetic_configure(window);

    list_add(&window_entry->head, &window->tag->tiled.windows);
    window->tag->tiled.focus = window->tag->tiled.windows.next;

    if (tag == window->tag)
    {
        arrange();

        xcb_map_window(c, window->window_id);

        property_values[0] = XCB_WM_STATE_NORMAL;
        property_values[1] = 0;
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id,
            WM_STATE, WM_STATE, 32, 2, property_values);

        focus(window->window_id);

        run_hooks(window, VELOX_HOOK_MANAGE_POST);
    }
}

void unmanage(struct velox_window_entry * entry)
{
    /* Deal with special cases */
    if (list_is_singular(&entry->window->tag->tiled.windows))
    {
        /* Set focus to the list head (no focus) */
        entry->window->tag->tiled.focus = &entry->window->tag->tiled.windows;
    }
    else if (&entry->head == entry->window->tag->tiled.focus)
    {
        entry->window->tag->tiled.focus = list_actual_next(tag->tiled.focus, &tag->tiled.windows);
    }

    list_del(&entry->head);

    if (tag == entry->window->tag)
    {
        if (list_empty(&tag->tiled.windows))
        {
            focus(root);
        }
        else
        {
            focus(list_entry(
                    entry->window->tag->tiled.focus, struct velox_window_entry, head
                )->window->window_id);
        }

        arrange();
    }

    run_hooks(entry->window, VELOX_HOOK_UNMANAGE);

    free(entry->window);
    free(entry);
}

void manage_existing_windows()
{
    DEBUG_ENTER

    xcb_query_tree_cookie_t query_cookie;
    xcb_query_tree_reply_t * query_reply;
    xcb_window_t * children;
    uint16_t child, child_count;
    xcb_get_window_attributes_cookie_t * window_attributes_cookies;
    xcb_get_window_attributes_reply_t ** window_attributes_replies;
    xcb_get_property_cookie_t * property_cookies;
    xcb_get_property_reply_t ** property_replies;
    xcb_get_property_cookie_t * state_cookies;
    xcb_get_property_reply_t ** state_replies;

    query_cookie = xcb_query_tree(c, root);
    query_reply = xcb_query_tree_reply(c, query_cookie, NULL);
    children = xcb_query_tree_children(query_reply);
    child_count = xcb_query_tree_children_length(query_reply);

    window_attributes_cookies = (xcb_get_window_attributes_cookie_t *)
        malloc(child_count * sizeof(xcb_get_window_attributes_cookie_t));
    window_attributes_replies = (xcb_get_window_attributes_reply_t **)
        malloc(child_count * sizeof(xcb_get_window_attributes_reply_t *));
    property_cookies = (xcb_get_property_cookie_t *)
        malloc(child_count * sizeof(xcb_get_property_cookie_t));
    property_replies = (xcb_get_property_reply_t **)
        malloc(child_count * sizeof(xcb_get_property_reply_t *));
    state_cookies = (xcb_get_property_cookie_t *)
        malloc(child_count * sizeof(xcb_get_property_cookie_t));
    state_replies = (xcb_get_property_reply_t **)
        malloc(child_count * sizeof(xcb_get_property_reply_t *));

    DEBUG_PRINT("child_count: %i\n", child_count)

    for (child = 0; child < child_count; child++)
    {
        window_attributes_cookies[child] = xcb_get_window_attributes(c, children[child]);
        property_cookies[child] = xcb_get_property(c, false, children[child],
            XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 1);
        state_cookies[child] = xcb_get_property(c, false, children[child],
            WM_STATE, WM_STATE, 0, 2);
    }
    for (child = 0; child < child_count; child++)
    {
        window_attributes_replies[child] = xcb_get_window_attributes_reply(c,
            window_attributes_cookies[child], NULL);
        property_replies[child] = xcb_get_property_reply(c, property_cookies[child], NULL);
        state_replies[child] = xcb_get_property_reply(c, state_cookies[child], NULL);

        if (window_attributes_replies[child]->override_redirect ||
            *((xcb_window_t *) xcb_get_property_value(property_replies[child])))
        {
            DEBUG_PRINT("override_redirect or transient\n")
            continue;
        }

        if (window_attributes_replies[child]->map_state == XCB_MAP_STATE_VIEWABLE ||
            ((uint32_t *) xcb_get_property_value(state_replies[child]))[0] == XCB_WM_STATE_ICONIC)
        {
            manage(children[child]);
        }
    }
    for (child = 0; child < child_count; child++)
    {
        if (*((xcb_window_t *) xcb_get_property_value(property_replies[child])) &&
            (window_attributes_replies[child]->map_state == XCB_MAP_STATE_VIEWABLE ||
            ((uint32_t *) xcb_get_property_value(state_replies[child]))[0] == XCB_WM_STATE_ICONIC))
        {
            manage(children[child]);
        }

        free(window_attributes_replies[child]);
        free(property_replies[child]);
        free(state_replies[child]);
    }

    free(query_reply);
    free(window_attributes_cookies);
    free(property_cookies);
    free(state_cookies);
    free(window_attributes_replies);
    free(property_replies);
    free(state_replies);
}

void spawn(char * const command[])
{
    DEBUG_ENTER

    if (fork() == 0)
    {
        if (c) close(xcb_get_file_descriptor(c));

        setsid();
        execvp(command[0], command);
        exit(0);
    }
}

void run()
{
    xcb_generic_event_t * event;

    printf("\n** Main Event Loop **\n");

    while (running && (event = xcb_wait_for_event(c)))
    {
        handle_event(event);

        if (clear_event_type)
        {
            xcb_aux_sync(c);

            while ((event = xcb_poll_for_event(c)))
            {
                if ((event->response_type & ~0x80) == clear_event_type)
                {
                    free(event);
                    DEBUG_PRINT("dropping masked event\n")
                }
                else
                {
                    handle_event(event);
                }
            }
            clear_event_type = 0;
        }
    }

}

void quit()
{
    running = false;
}

void cleanup()
{
    cleanup_ewmh();
    cleanup_modules();
    cleanup_key_bindings();
    cleanup_tags();
    cleanup_layouts();
    cleanup_work_area_modifiers();
    cleanup_hooks();

    /* X cursors */
    if (!xcb_connection_has_error(c))
    {
        xcb_free_cursor(c, cursors[POINTER]);
        xcb_free_cursor(c, cursors[RESIZE]);
        xcb_free_cursor(c, cursors[MOVE]);

        /* X colors */
        {
            uint32_t pixels[] = { border_pixel, border_focus_pixel };

            xcb_free_colors(c, screen->default_colormap, 0, sizeof(pixels) / 4, pixels);
        }

        xcb_disconnect(c);
    }
}

void __attribute__((noreturn)) die(const char const * message, ...)
{
    va_list args;

    va_start(args, message);
    fputs("FATAL: ", stderr);
    vfprintf(stderr, message, args);
    fputc('\n', stderr);
    va_end(args);

    cleanup();

    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv)
{
    srand(time(NULL));

    printf("Velox Window Manager\n");

    setup();
    manage_existing_windows();
    run_hooks(NULL, VELOX_HOOK_STARTUP);
    run();
    cleanup();

    return EXIT_SUCCESS;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

