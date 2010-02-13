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
xcb_get_keyboard_mapping_reply_t * keyboard_mapping;

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
const uint16_t mod_mask_numlock = XCB_MOD_MASK_2;

void cleanup_windows()
{
    struct velox_tag * tag;

    while (tags != NULL)
    {
        tag = (struct velox_tag *) tags->data;
        velox_loop_delete(tag->windows, true);
        free(tag);

        tags = velox_list_remove_first(tags);
    }

}

struct velox_window * tags_lookup_window(xcb_window_t window_id)
{
    struct velox_list * iterator;
    struct velox_tag * tag;
    struct velox_window * window;

    for (iterator = tags; iterator != NULL; iterator = iterator->next)
    {
        tag = (struct velox_tag *) iterator->data;
        window = window_loop_lookup(tag->windows, window_id);

        if (window != NULL)
        {
            assert(window->tag == tag);
            return window;
        }
    }

    return NULL;
}

void grab_keys(xcb_keycode_t min_keycode, xcb_keycode_t max_keycode)
{
        xcb_get_keyboard_mapping_cookie_t keyboard_mapping_cookie;
        xcb_keysym_t * keysyms;
        struct velox_list * iterator;
        struct velox_key_binding * binding;
        uint16_t keysym_index;
        uint16_t extra_modifier_index;
        uint16_t extra_modifiers[] = {
            0,
            mod_mask_numlock,
            XCB_MOD_MASK_LOCK,
            mod_mask_numlock | XCB_MOD_MASK_LOCK
        };
        uint16_t extra_modifiers_count = sizeof(extra_modifiers) / sizeof(uint16_t);

        printf("grabbing keys\n");

        keyboard_mapping_cookie = xcb_get_keyboard_mapping(c, min_keycode, max_keycode - min_keycode + 1);

        xcb_ungrab_key(c, XCB_GRAB_ANY, root, XCB_MOD_MASK_ANY);

        free(keyboard_mapping);
        keyboard_mapping = xcb_get_keyboard_mapping_reply(c, keyboard_mapping_cookie, NULL);
        keysyms = xcb_get_keyboard_mapping_keysyms(keyboard_mapping);
        for (iterator = key_bindings; iterator != NULL; iterator = iterator->next)
        {
            binding = (struct velox_key_binding *) iterator->data;

            for (keysym_index = 0; keysym_index < xcb_get_keyboard_mapping_keysyms_length(keyboard_mapping); keysym_index++)
            {
                if (keysyms[keysym_index] == binding->key->keysym)
                {
                    binding->keycode = min_keycode + (keysym_index / keyboard_mapping->keysyms_per_keycode);
                    break;
                }
            }

            for (extra_modifier_index = 0; extra_modifier_index < extra_modifiers_count; extra_modifier_index++)
            {
                xcb_grab_key(c, true, root,
                    binding->key->modifiers | extra_modifiers[extra_modifier_index],
                    binding->keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
                );
            }
        }

        xcb_flush(c);
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

    xcb_create_glyph_cursor(c, cursors[POINTER], cursor_font, cursor_font, POINTER_ID, POINTER_ID + 1, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF);
    xcb_create_glyph_cursor(c, cursors[RESIZE], cursor_font, cursor_font, RESIZE_ID, RESIZE_ID + 1, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF);
    xcb_create_glyph_cursor(c, cursors[MOVE], cursor_font, cursor_font, MOVE_ID, MOVE_ID + 1, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF);

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

    setup_configured_keys();

    parse_config();

    setup_layouts();
    setup_key_bindings();
    setup_hooks();

    initialize_modules();

    setup_tags();
    setup_ewmh();

    grab_keys(setup->min_keycode, setup->max_keycode);

    tag = (struct velox_tag *) tags->data;
}

void show_window(xcb_window_t window_id)
{
    uint32_t property_values[2];

    printf("show_window: %i\n", window_id);

    property_values[0] = XCB_WM_STATE_NORMAL;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window_id, WM_STATE, WM_STATE, 32, 2, property_values);

    xcb_map_window(c, window_id);
}

void hide_window(xcb_window_t window_id)
{
    uint32_t property_values[2];

    printf("hide_window: %i\n", window_id);

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

    xcb_send_event(c, false, root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char *) &event);
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
    xcb_flush(c);
}

void set_tag(uint8_t index)
{
    struct velox_list * tag_iterator;
    struct velox_tag * new_tag;

    for (tag_iterator = tags; tag_iterator != NULL && index > 1; tag_iterator = tag_iterator->next, --index);
    new_tag = (struct velox_tag *) tag_iterator->data;

    printf("set_tag: %u\n", (uint32_t) new_tag);

    if (tag == new_tag) return; // Nothing to do...
    else
    {
        struct velox_loop * iterator;
        struct velox_window * window;

        if (new_tag->windows)
        {
            /* Show the windows now visible */
            iterator = new_tag->windows;
            do
            {
                show_window(((struct velox_window *) iterator->data)->window_id);
                iterator = iterator->next;
            } while (iterator != new_tag->windows);
        }

        if (tag->windows)
        {
            /* Hide windows no longer visible */
            iterator = tag->windows;
            do
            {
                hide_window(((struct velox_window *) iterator->data)->window_id);
                iterator = iterator->next;
            } while (iterator != tag->windows);
        }

        tag = new_tag;

        if (tag->windows)
        {
            assert(new_tag->focus != NULL);
            focus(((struct velox_window *) new_tag->focus->data)->window_id);
        }
        else
        {
            focus(root);
        }

        arrange();

        run_hooks(tag, VELOX_HOOK_TAG_CHANGED);
    }
}

void move_focus_to_tag(uint8_t index)
{
    struct velox_list * tag_iterator;
    struct velox_tag * new_tag;

    for (tag_iterator = tags; tag_iterator != NULL && index > 1; tag_iterator = tag_iterator->next, --index);
    new_tag = (struct velox_tag *) tag_iterator->data;

    printf("move_focus_to_tag: %i\n", (uint32_t) new_tag);

    if (tag->windows == NULL)
    {
        return;
    }
    else
    {
        struct velox_loop * next_focus;
        struct velox_window * window;

        window = (struct velox_window *) tag->focus->data;

        window->tag = new_tag;

        new_tag->windows = velox_loop_insert(new_tag->windows, tag->focus->data);

        if (velox_loop_is_singleton(new_tag->windows))
        {
            new_tag->focus = new_tag->windows;
        }

        /* If we removed the first element */
        if (tag->focus == tag->windows)
        {
            tag->focus = velox_loop_remove(tag->focus);
            tag->windows = tag->focus;
        }
        else
        {
            tag->focus = velox_loop_remove(tag->focus);
        }

        if (tag->focus)
        {
            focus(((struct velox_window *) tag->focus->data)->window_id);
        }
        else
        {
            focus(root);
        }

        hide_window(window->window_id);

        arrange();
    }

    xcb_flush(c);
}

void next_layout()
{
    printf("next_layout()\n");

    tag->layout = tag->layout->next;
    tag->state = ((struct velox_layout *) tag->layout->data)->default_state;

    arrange();
}

void previous_layout()
{
    printf("next_layout()\n");

    tag->layout = tag->layout->previous;
    tag->state = ((struct velox_layout *) tag->layout->data)->default_state;

    arrange();
}

void focus_next()
{
    printf("focus_next()\n");

    if (tag->focus == NULL)
    {
        return;
    }

    tag->focus = tag->focus->next;

    focus(((struct velox_window *) tag->focus->data)->window_id);
}

void focus_previous()
{
    printf("focus_previous()\n");

    if (tag->focus == NULL)
    {
        return;
    }

    tag->focus = tag->focus->previous;

    focus(((struct velox_window *) tag->focus->data)->window_id);
}

void move_next()
{
    printf("move_next()\n");

    if (tag->focus == NULL)
    {
        return;
    }

    velox_loop_swap(tag->focus, tag->focus->next);
    tag->focus = tag->focus->next;

    arrange();
}

void move_previous()
{
    printf("move_previous()\n");

    if (tag->focus == NULL)
    {
        return;
    }

    velox_loop_swap(tag->focus, tag->focus->previous);
    tag->focus = tag->focus->previous;

    arrange();
}

void kill_focused_window()
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (focus_reply->focus == root)
    {
        return;
    }

    printf("killing focused window\n");

    if (window_has_protocol(focus_reply->focus, WM_DELETE_WINDOW))
    {
        xcb_client_message_event_t event;

        printf("wm_delete\n");

        event.response_type = XCB_CLIENT_MESSAGE;
        event.format = 32;
        event.window = focus_reply->focus;
        event.type = WM_PROTOCOLS;
        event.data.data32[0] = WM_DELETE_WINDOW;
        event.data.data32[1] = XCB_CURRENT_TIME;

        xcb_send_event(c, false, focus_reply->focus, XCB_EVENT_MASK_NO_EVENT, (char *) &event);
    }
    else
    {
        printf("xcb_kill_client\n");

        xcb_kill_client(c, focus_reply->focus);
    }

    xcb_flush(c);
}

void arrange()
{
    printf("arrange()\n");
    printf("tag: %i\n", (uint32_t) tag);

    if (tag->windows == NULL) return;

    assert(tag->layout->data != NULL);

    calculate_work_area(&screen_area, &work_area);
    ((struct velox_layout *) tag->layout->data)->arrange(&work_area, tag->windows, &tag->state);

    clear_event_type = XCB_ENTER_NOTIFY;
}

void update_name_class(struct velox_window * window)
{
    xcb_get_property_cookie_t wm_name_cookie, wm_class_cookie;
    xcb_get_property_reply_t * wm_name_reply, * wm_class_reply;

    wm_name_cookie = xcb_get_property(c, false, window->window_id, XCB_ATOM_WM_NAME, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);
    wm_class_cookie = xcb_get_property(c, false, window->window_id, XCB_ATOM_WM_CLASS, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);

    wm_name_reply = xcb_get_property_reply(c, wm_name_cookie, NULL);
    wm_class_reply = xcb_get_property_reply(c, wm_class_cookie, NULL);

    printf("wm_name: %s\n", xcb_get_property_value(wm_name_reply));
    printf("wm_class: %s\n", xcb_get_property_value(wm_class_reply));

    window->name = strndup(xcb_get_property_value(wm_name_reply), xcb_get_property_value_length(wm_name_reply));
    window->class = strndup(xcb_get_property_value(wm_class_reply), xcb_get_property_value_length(wm_class_reply));

    free(wm_name_reply);
    free(wm_class_reply);
}

void manage(xcb_window_t window_id)
{
    printf("manage(%i)\n", window_id);

    struct velox_window * window = NULL;
    struct velox_window * transient = NULL;
    xcb_get_property_cookie_t transient_for_cookie;
    xcb_get_property_reply_t * transient_for_reply = NULL;
    xcb_get_geometry_cookie_t geometry_cookie;
    xcb_get_geometry_reply_t * geometry = NULL;
    xcb_window_t transient_id = 0;
    uint32_t mask;
    uint32_t values[2];
    uint32_t property_values[2];

    transient_for_cookie = xcb_get_property(c, false, window_id, XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 1);
    geometry_cookie = xcb_get_geometry(c, window_id);

    window = (struct velox_window *) malloc(sizeof(struct velox_window));
    printf("allocated window: %i\n", (uint32_t) window);

    window->window_id = window_id;

    transient_for_reply = xcb_get_property_reply(c, transient_for_cookie, NULL);
    transient_id = *((xcb_window_t *) xcb_get_property_value(transient_for_reply));

    if (transient_for_reply->type == XCB_ATOM_WINDOW && transient_id)
    {
        printf("transient_id: %i\n", transient_id);
        transient = tags_lookup_window(transient_id);

        window->floating = true;
    }
    else
    {
        window->floating = false;
    }

    if (transient != NULL)
    {
        window->tag = transient->tag;
    }
    else
    {
        window->tag = tag;
    }

    free(transient_for_reply);

    /* Geometry */
    geometry = xcb_get_geometry_reply(c, geometry_cookie, NULL);

    printf("x: %i, y: %i, width: %i, height: %i\n", geometry->x, geometry->y, geometry->width, geometry->height);

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

    if (tag == window->tag)
    {
        tag->windows = velox_loop_insert(tag->windows, window)->previous;
        tag->focus = tag->windows;

        arrange();

        xcb_map_window(c, window->window_id);

        property_values[0] = XCB_WM_STATE_NORMAL;
        property_values[1] = 0;
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id, WM_STATE, WM_STATE, 32, 2, property_values);

        focus(window->window_id);

        run_hooks(window, VELOX_HOOK_MANAGE_POST);
    }
    else
    {
        window->tag->windows = velox_loop_insert(window->tag->windows, window)->previous;
        window->tag->focus = window->tag->windows;
    }
}

void unmanage(struct velox_window * window)
{
    struct velox_loop * iterator;

    iterator = window->tag->windows;
    if (iterator == NULL) return;
    do
    {
        if (iterator->data == window)
        {
            if (iterator == window->tag->focus)
            {
                if (velox_loop_is_singleton(window->tag->windows))
                {
                    window->tag->focus = NULL;
                }
                else
                {
                    window->tag->focus = window->tag->focus->next;
                }
            }

            if (iterator == window->tag->windows)
            {
                iterator = velox_loop_remove(iterator);
                window->tag->windows = iterator;
            }
            else
            {
                iterator = velox_loop_remove(iterator);
            }

            if (tag == window->tag)
            {
                if (tag->focus)
                {
                    focus(((struct velox_window *) tag->focus->data)->window_id);
                }
                else
                {
                    focus(root);
                }

                arrange();
            }

            run_hooks(window, VELOX_HOOK_UNMANAGE);

            free(window);

            return;
        }

        iterator = iterator->next;
    } while (iterator != window->tag->windows);
}

void manage_existing_windows()
{
    printf("manage_existing_windows()\n");

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

    window_attributes_cookies = (xcb_get_window_attributes_cookie_t *) malloc(child_count * sizeof(xcb_get_window_attributes_cookie_t));
    window_attributes_replies = (xcb_get_window_attributes_reply_t **) malloc(child_count * sizeof(xcb_get_window_attributes_reply_t *));
    property_cookies = (xcb_get_property_cookie_t *) malloc(child_count * sizeof(xcb_get_property_cookie_t));
    property_replies = (xcb_get_property_reply_t **) malloc(child_count * sizeof(xcb_get_property_reply_t *));
    state_cookies = (xcb_get_property_cookie_t *) malloc(child_count * sizeof(xcb_get_property_cookie_t));
    state_replies = (xcb_get_property_reply_t **) malloc(child_count * sizeof(xcb_get_property_reply_t *));

    printf("child_count: %i\n", child_count);

    for (child = 0; child < child_count; child++)
    {
        window_attributes_cookies[child] = xcb_get_window_attributes(c, children[child]);
        property_cookies[child] = xcb_get_property(c, false, children[child], XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 1);
        state_cookies[child] = xcb_get_property(c, false, children[child], WM_STATE, WM_STATE, 0, 2);
    }
    for (child = 0; child < child_count; child++)
    {
        window_attributes_replies[child] = xcb_get_window_attributes_reply(c, window_attributes_cookies[child], NULL);
        property_replies[child] = xcb_get_property_reply(c, property_cookies[child], NULL);
        state_replies[child] = xcb_get_property_reply(c, state_cookies[child], NULL);

        if (window_attributes_replies[child]->override_redirect || *((xcb_window_t *) xcb_get_property_value(property_replies[child])))
        {
            printf("override_redirect or transient\n");
            continue;
        }

        printf("map_state: %i\n", window_attributes_replies[child]->map_state);

        printf("state: %i\n", ((uint32_t *) xcb_get_property_value(state_replies[child]))[0]);

        if (window_attributes_replies[child]->map_state == XCB_MAP_STATE_VIEWABLE || ((uint32_t *) xcb_get_property_value(state_replies[child]))[0] == XCB_WM_STATE_ICONIC)
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
    if (fork() == 0)
    {
        if (c) close(xcb_get_file_descriptor(c));

        setsid();
        printf("executing\n");
        execvp(command[0], command);
        exit(0);
    }
}

void run()
{
    xcb_generic_event_t * event;
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
                    printf("dropping masked event\n");
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
    cleanup_windows();
    cleanup_tags();
    cleanup_layouts();
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

void die(const char const * message, ...)
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

    setup();
    manage_existing_windows();
    run_hooks(NULL, VELOX_HOOK_STARTUP);
    run();
    cleanup();

    return 0;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

