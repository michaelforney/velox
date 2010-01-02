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
#include <assert.h>
#include <limits.h>

#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_aux.h>

#include <X11/keysym.h>

#include "mwm.h"
#include "window.h"
#include "tag.h"
#include "hook.h"
#include "keybinding.h"

/* X variables */
xcb_connection_t * c;
xcb_screen_t * screen;
xcb_window_t root;
xcb_get_keyboard_mapping_reply_t * keyboard_mapping;

uint16_t screen_width = 0;
uint16_t screen_height = 0;

/* X atoms */
const uint16_t atom_length = 5;
xcb_atom_t WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE;
xcb_atom_t _NET_SUPPORTED, _NET_WM_NAME;

/* X cursors */
enum
{
    POINTER_ID = 56,
    RESIZE_ID = 120,
    MOVE_ID = 52
};
enum
{
    POINTER,
    RESIZE,
    MOVE
};

xcb_cursor_t cursors[3];

/* MWM variables */
bool running = true;
uint64_t tag_mask = 0;
struct mwm_window_list * visible_windows = NULL;
struct mwm_window_list * hidden_windows = NULL;
struct mwm_tag * main_tag = NULL;
uint16_t pending_unmaps = 0;
uint8_t clear_event_type = 0;

uint32_t border_pixel;
uint32_t border_focus_pixel;

/* MWM constants */
const uint16_t border_color[] = { 0x9999, 0x9999, 0x9999 };
const uint16_t border_focus_color[] = { 0x3333,  0x8888, 0x3333 };
const uint16_t border_width = 2;
const uint16_t mod_mask_numlock = XCB_MOD_MASK_2;

/* MWM macros */
#define CLEAN_MASK(mask) (mask & ~(mod_mask_numlock | XCB_MOD_MASK_LOCK))

void grab_keys(xcb_keycode_t min_keycode, xcb_keycode_t max_keycode)
{
        xcb_get_keyboard_mapping_cookie_t keyboard_mapping_cookie;
        xcb_keysym_t * keysyms;
        struct mwm_key_binding_list * current_element;
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
        for (current_element = key_bindings; current_element != NULL; current_element = current_element->next)
        {
            for (keysym_index = 0; keysym_index < xcb_get_keyboard_mapping_keysyms_length(keyboard_mapping); keysym_index++)
            {
                if (keysyms[keysym_index] == current_element->binding.keysym)
                {
                    current_element->binding.keycode = min_keycode + (keysym_index / keyboard_mapping->keysyms_per_keycode);
                    break;
                }
            }

            for (extra_modifier_index = 0; extra_modifier_index < extra_modifiers_count; extra_modifier_index++)
            {
                xcb_grab_key(c, true, root,
                    current_element->binding.modifiers | extra_modifiers[extra_modifier_index],
                    current_element->binding.keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
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
        fprintf(stderr, "mwm: another window manager is already running\n");
        exit(EXIT_FAILURE);
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

    if (xcb_connection_has_error(c))
    {
        fprintf(stderr, "mwm: could not open display\n");
        exit(EXIT_FAILURE);
    }

    setup = xcb_get_setup(c);

    screen_iterator = xcb_setup_roots_iterator(setup);

    screen = screen_iterator.data;
    root = screen->root;
    screen_width = screen->width_in_pixels;
    screen_height = screen->height_in_pixels;

    check_wm_running();

    /* Allocate colors */
    border_color_cookie = xcb_alloc_color(c, screen->default_colormap, border_color[0], border_color[1], border_color[2]);
    border_focus_color_cookie = xcb_alloc_color(c, screen->default_colormap, border_focus_color[0], border_focus_color[1], border_focus_color[2]);

    /* Setup atoms */
    atom_cookies = (xcb_intern_atom_cookie_t *) malloc(atom_length * sizeof(xcb_intern_atom_cookie_t));
    atom_cookies[0] = xcb_intern_atom(c, false, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
    atom_cookies[1] = xcb_intern_atom(c, false, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    atom_cookies[2] = xcb_intern_atom(c, false, strlen("WM_STATE"), "WM_STATE");
    atom_cookies[3] = xcb_intern_atom(c, false, strlen("_NET_SUPPORTED"), "_NET_SUPPORTED");
    atom_cookies[4] = xcb_intern_atom(c, false, strlen("_NET_WM_NAME"), "_NET_WM_NAME");

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
    _NET_SUPPORTED = atom_reply->atom;
    free(atom_reply);
    atom_reply = xcb_intern_atom_reply(c, atom_cookies[4], NULL);
    _NET_WM_NAME = atom_reply->atom;
    free(atom_reply);

    free(atom_cookies);

    {
        xcb_atom_t net_atoms[] = { _NET_SUPPORTED, _NET_WM_NAME };

        xcb_change_property(c, XCB_PROP_MODE_REPLACE, root, _NET_SUPPORTED, ATOM, 32, sizeof(net_atoms) / sizeof(xcb_atom_t), net_atoms);
    }

    setup_layouts();
    setup_tags();
    setup_key_bindings();

    grab_keys(setup->min_keycode, setup->max_keycode);

    main_tag = &tags[TERM];
    tag_mask = main_tag->id;
}

void show_window(struct mwm_window * window)
{
    uint32_t property_values[2];

    printf("show_window: %i\n", window->window_id);

    property_values[0] = XCB_WM_STATE_NORMAL;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id, WM_STATE, WM_STATE, 32, 2, property_values);

    xcb_map_window(c, window->window_id);
}

void hide_window(struct mwm_window * window)
{
    uint32_t property_values[2];

    printf("hide_window: %i\n", window->window_id);

    property_values[0] = XCB_WM_STATE_WITHDRAWN; // FIXME: Maybe this should be iconic?
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id, WM_STATE, WM_STATE, 32, 2, property_values);

    pending_unmaps++;

    xcb_unmap_window(c, window->window_id);
}

/**
 * Sends a synthetic configure request to the window
 *
 * @param window The window to send the request to
 */
void synthetic_configure(struct mwm_window * window)
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
void synthetic_unmap(struct mwm_window * window)
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

    main_tag->focus = window_id;

    if (focus_reply->focus == window_id)
    {
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

void toggle_tag(struct mwm_tag * tag)
{
    // TODO: Implement
}

void set_tag(struct mwm_tag * tag)
{
    struct mwm_window_list * current_element = NULL;
    struct mwm_window_list * previous_element = NULL;
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;
    uint16_t windows_hidden = 0;
    uint16_t windows_shown = 0;
    uint16_t window_index;
    bool hid_focus = false;

    if (main_tag == tag && tag_mask == tag->id)
    {
        return; // Nothing to do...
    }

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    printf("set_tag: %i\n", tag);

    main_tag = tag;
    tag_mask = tag->id;

    /* Hide windows no longer visible */
    while (visible_windows != NULL && !(visible_windows->window->tags & tag_mask))
    {
        struct mwm_window_list * new_visible_windows = NULL;

        if (visible_windows->window->window_id == focus_reply->focus)
        {
            hid_focus = true;
        }

        hidden_windows = window_list_insert(hidden_windows, visible_windows->window);

        new_visible_windows = visible_windows->next;
        free(visible_windows);
        visible_windows = new_visible_windows;

        windows_hidden++;
    }

    if (visible_windows != NULL)
    {
        for (previous_element = visible_windows, current_element = visible_windows->next; current_element != NULL; )
        {
            if (!current_element->window->tags & tag_mask)
            {
                if (visible_windows->window->window_id == focus_reply->focus)
                {
                    hid_focus = true;
                }

                hidden_windows = window_list_insert(hidden_windows, current_element->window);

                previous_element->next = current_element->next;
                free(current_element);

                current_element = previous_element->next;

                windows_hidden++;
            }
            else
            {
                previous_element = previous_element->next;
                current_element = current_element->next;
            }
        }
    }

    /* Show previously hidden windows */
    while (hidden_windows != NULL && hidden_windows->window->tags & tag_mask)
    {
        struct mwm_window_list * new_hidden_windows = NULL;

        visible_windows = window_list_insert(visible_windows, hidden_windows->window);

        new_hidden_windows = hidden_windows->next;
        free(hidden_windows);
        hidden_windows = new_hidden_windows;

        windows_shown++;
    }

    if (hidden_windows != NULL)
    {
        for (previous_element = hidden_windows, current_element = hidden_windows->next; current_element != NULL; )
        {
            if (current_element->window->tags & tag_mask)
            {
                visible_windows = window_list_insert(visible_windows, current_element->window);

                previous_element->next = current_element->next;
                free(current_element);

                current_element = previous_element->next;

                windows_shown++;
            }
            else
            {
                previous_element = previous_element->next;
                current_element = current_element->next;
            }
        }
    }

    /* Show previously hidden windows */
    for (current_element = visible_windows, window_index = 0; window_index < windows_shown; current_element = current_element->next, window_index++)
    {
        show_window(current_element->window);
    }

    /* Hide previously shown windows */
    for (current_element = hidden_windows, window_index = 0; window_index < windows_hidden; current_element = current_element->next, window_index++)
    {
        hide_window(current_element->window);
    }

    if (hid_focus || focus_reply->focus == root)
    {
        printf("tag's focus: %i\n", main_tag->focus);

        if (window_list_lookup(visible_windows, main_tag->focus) != NULL)
        {
            focus(main_tag->focus);
        }
        else
        {
            if (visible_windows)
            {
                focus(visible_windows->window->window_id);
            }
            else
            {
                focus(root);
            }
        }
    }

    arrange();
}

void move_focus_to_tag(struct mwm_tag * tag)
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    printf("move_focus_to_tag\n");

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (focus_reply->focus == root || !visible_windows)
    {
        return;
    }

    if (visible_windows->window->window_id == focus_reply->focus)
    {
        visible_windows->window->tags = tag->id;

        if (!(visible_windows->window->tags & tag_mask))
        {
            struct mwm_window_list * new_list = visible_windows->next;

            if (visible_windows->next)
            {
                focus(visible_windows->next->window->window_id);
            }
            else
            {
                focus(root);
            }

            hide_window(visible_windows->window);

            hidden_windows = window_list_insert(hidden_windows, visible_windows->window);

            free(visible_windows);
            visible_windows = new_list;

            arrange();
        }
    }
    else
    {
        struct mwm_window_list * current_element, * previous_element;

        for (previous_element = visible_windows, current_element = visible_windows->next; current_element != NULL; previous_element = current_element, current_element = current_element->next)
        {
            if (current_element->window->window_id == focus_reply->focus)
            {
                current_element->window->tags = tag->id;

                if (!(current_element->window->tags & tag_mask))
                {
                    if (current_element->next)
                    {
                        focus(current_element->next->window->window_id);
                    }
                    else if (visible_windows != current_element)
                    {
                        focus(visible_windows->window->window_id);
                    }
                    else
                    {
                        focus(root);
                    }

                    hide_window(current_element->window);

                    hidden_windows = window_list_insert(hidden_windows, current_element->window);
                    previous_element->next = current_element->next;
                    free(current_element);

                    arrange();
                }

                break;
            }
        }
    }

    xcb_flush(c);
}

void next_layout()
{
    printf("next_layout()\n");

    main_tag->layout_index++;
    if (main_tag->layouts[main_tag->layout_index] == NULL)
    {
        main_tag->layout_index = 0;
    }

    printf("next index: %i\n", main_tag->layout_index);

    assert(main_tag->layouts[main_tag->layout_index]);

    main_tag->state = main_tag->layouts[main_tag->layout_index]->default_state;

    arrange();
}

void previous_layout()
{
    printf("next_layout()\n");

    if (main_tag->layout_index == 0)
    {
        while (main_tag->layouts[main_tag->layout_index + 1] != NULL)
        {
            main_tag->layout_index++;
        }
    }
    else
    {
        main_tag->layout_index--;
    }

    printf("next index: %i\n", main_tag->layout_index);

    assert(main_tag->layouts[main_tag->layout_index]);

    main_tag->state = main_tag->layouts[main_tag->layout_index]->default_state;

    arrange();
}

void focus_next()
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    printf("focus_next()\n");

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (focus_reply->focus == root)
    {
        if (visible_windows)
        {
            focus(visible_windows->window->window_id);
        }
    }
    else
    {
        struct mwm_window_list * current_element;

        for (current_element = visible_windows; current_element != NULL; current_element = current_element->next)
        {
            if (current_element->window->window_id == focus_reply->focus)
            {
                xcb_window_t next_window_id;

                if (current_element->next != NULL)
                {
                    next_window_id = current_element->next->window->window_id;
                }
                else
                {
                    next_window_id = visible_windows->window->window_id;
                }

                focus(next_window_id);

                break;
            }
        }
    }
}

void focus_previous()
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    printf("focus_previous()\n");

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (focus_reply->focus == root)
    {
        if (visible_windows)
        {
            focus(visible_windows->window->window_id);
        }
    }
    else
    {
        struct mwm_window_list * previous_element;
        struct mwm_window_list * current_element;

        assert(visible_windows);
        if (visible_windows->window->window_id == focus_reply->focus)
        {
            for (current_element = visible_windows; current_element->next != NULL; current_element = current_element->next);
            focus(current_element->window->window_id);
        }
        else
        {
            for (previous_element = visible_windows, current_element = visible_windows->next; current_element != NULL; previous_element = current_element, current_element = current_element->next)
            {
                if (current_element->window->window_id == focus_reply->focus)
                {
                    xcb_window_t next_window_id;

                    focus(previous_element->window->window_id);

                    break;
                }
            }
        }
    }
}

void move_next()
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    printf("move_next()\n");

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (visible_windows && visible_windows->next && focus_reply->focus != root)
    {
        struct mwm_window_list * current_element;

        if (visible_windows->window->window_id == focus_reply->focus)
        {
            struct mwm_window * first_window = visible_windows->window;

            visible_windows->window = visible_windows->next->window;
            visible_windows->next->window = first_window;
        }
        else
        {
            for (current_element = visible_windows; current_element != NULL; current_element = current_element->next)
            {
                if (current_element->window->window_id == focus_reply->focus)
                {
                    struct mwm_window_list * next_element;
                    struct mwm_window * next_window;

                    if (current_element->next != NULL)
                    {
                        next_element = current_element->next;
                    }
                    else
                    {
                        next_element = visible_windows;
                    }

                    next_window = next_element->window;
                    next_element->window = current_element->window;
                    current_element->window = next_window;

                    break;
                }
            }
        }
        arrange();
    }
}

void move_previous()
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    printf("move_previous()\n");

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (visible_windows && visible_windows->next && focus_reply->focus != root) /* There must be two visible windows for this to make any sense */
    {
        struct mwm_window_list * current_element;
        struct mwm_window_list * previous_element;

        if (visible_windows->window->window_id == focus_reply->focus)
        {
            struct mwm_window * current_window = visible_windows->window;

            for (previous_element = visible_windows; previous_element->next != NULL; previous_element = previous_element->next);

            visible_windows->window = previous_element->window;
            previous_element->window = current_window;
        }
        else
        {
            for (previous_element = visible_windows, current_element = visible_windows->next; current_element != NULL; previous_element = current_element, current_element = current_element->next)
            {
                if (current_element->window->window_id == focus_reply->focus)
                {
                    struct mwm_window * current_window;

                    current_window = current_element->window;
                    current_element->window = previous_element->window;
                    previous_element->window = current_window;

                    break;
                }
            }
        }
        arrange();
    }
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

void increase_master_factor()
{
    printf("increase_master_factor()\n");

    if (main_tag->layouts[main_tag->layout_index] == &layouts[TILE])
    {
        struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) (&main_tag->state);
        state->master_factor = MIN(state->master_factor + 0.025, 1.0);

        arrange();
    }
}

void decrease_master_factor()
{
    printf("decrease_master_factor()\n");

    if (main_tag->layouts[main_tag->layout_index] == &layouts[TILE])
    {
        struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) (&main_tag->state);
        state->master_factor = MAX(state->master_factor - 0.025, 0.0);

        arrange();
    }
}

void increase_master_count()
{
    printf("increase_master_count()\n");

    if (main_tag->layouts[main_tag->layout_index] == &layouts[TILE])
    {
        struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) (&main_tag->state);
        state->master_count++;

        arrange();
    }
}

void decrease_master_count()
{
    printf("decrease_master_count()\n");

    if (main_tag->layouts[main_tag->layout_index] == &layouts[TILE])
    {
        struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) (&main_tag->state);
        state->master_count = MAX(state->master_count - 1, 0);

        arrange();
    }
}

void increase_column_count()
{
    printf("increase_column_count()\n");

    if (main_tag->layouts[main_tag->layout_index] == &layouts[TILE])
    {
        struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) (&main_tag->state);
        state->column_count++;

        arrange();
    }
}

void decrease_column_count()
{
    printf("decrease_column_count()\n");

    if (main_tag->layouts[main_tag->layout_index] == &layouts[TILE])
    {
        struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) (&main_tag->state);
        state->column_count = MAX(state->column_count - 1, 1);

        arrange();
    }
}

void arrange()
{
    printf("arrange()\n");
    printf("main_tag: %i, visible_windows: %i\n", main_tag, visible_windows);

    if (main_tag == NULL || visible_windows == NULL)
    {
        return;
    }

    assert(main_tag->layouts[main_tag->layout_index] != NULL);
    main_tag->layouts[main_tag->layout_index]->arrange(visible_windows, &main_tag->state);

    clear_event_type = XCB_ENTER_NOTIFY;
}

void update_name_class(struct mwm_window * window)
{
    xcb_get_property_cookie_t wm_name_cookie, wm_class_cookie;
    xcb_get_property_reply_t * wm_name_reply, * wm_class_reply;

    wm_name_cookie = xcb_get_property(c, false, window->window_id, WM_NAME, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);
    wm_class_cookie = xcb_get_property(c, false, window->window_id, WM_CLASS, XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX);

    wm_name_reply = xcb_get_property_reply(c, wm_name_cookie, NULL);
    wm_class_reply = xcb_get_property_reply(c, wm_class_cookie, NULL);

    printf("wm_name: %s\n", xcb_get_property_value(wm_name_reply));
    printf("wm_class: %s\n", xcb_get_property_value(wm_class_reply));

    window->name = strndup(xcb_get_property_value(wm_name_reply), xcb_get_property_value_length(wm_name_reply));
    window->class = strndup(xcb_get_property_value(wm_class_reply), xcb_get_property_value_length(wm_class_reply));
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
    uint32_t values[2];
    uint32_t property_values[2];

    transient_for_cookie = xcb_get_property(c, false, window_id, WM_TRANSIENT_FOR, WINDOW, 0, 1);
    geometry_cookie = xcb_get_geometry(c, window_id);

    window = (struct mwm_window *) malloc(sizeof(struct mwm_window));
    printf("allocated window: %i\n", window);

    window->window_id = window_id;

    transient_for_reply = xcb_get_property_reply(c, transient_for_cookie, NULL);
    transient_id = *((xcb_window_t *) xcb_get_property_value(transient_for_reply));

    if (transient_for_reply->type == WINDOW && transient_id)
    {
        printf("transient_id: %i\n", transient_id);
        transient = window_list_lookup(visible_windows, transient_id);
        if (transient == NULL)
        {
            transient = window_list_lookup(hidden_windows, transient_id);
        }

        window->floating = true;
    }
    else
    {
        window->floating = false;
    }

    if (transient != NULL)
    {
        window->tags = transient->tags;
    }
    else
    {
        window->tags = main_tag->id;
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

    update_name_class(window);

    run_manage_hooks(window);

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

    if (tag_mask & window->tags)
    {
        visible_windows = window_list_insert(visible_windows, window);

        arrange();

        xcb_map_window(c, window->window_id);

        property_values[0] = XCB_WM_STATE_NORMAL;
        property_values[1] = 0;
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id, WM_STATE, WM_STATE, 32, 2, property_values);

        focus(visible_windows->window->window_id);

    }
    else
    {
        hide_window(window);
        hidden_windows = window_list_insert(visible_windows, window);
    }
}

void unmanage(struct mwm_window * window)
{
    // FIXME: This should be done a better way
    if (window_list_lookup(visible_windows, window->window_id) != NULL)
    {
        visible_windows = window_list_delete(visible_windows, window->window_id);

        arrange();
    }
    else
    {
        hidden_windows = window_list_delete(hidden_windows, window->window_id);
    }

    free(window);

    if (visible_windows)
    {
        focus(visible_windows->window->window_id);
    }
    else
    {
        focus(root);
    }
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
    xcb_get_property_cookie_t * state_cookies;
    xcb_get_property_reply_t ** state_replies;

    query_cookie = xcb_query_tree(c, root);
    query = xcb_query_tree_reply(c, query_cookie, NULL);
    children = xcb_query_tree_children(query);
    child_count = xcb_query_tree_children_length(query);

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
        property_cookies[child] = xcb_get_property(c, false, children[child], WM_TRANSIENT_FOR, WINDOW, 0, 1);
        state_cookies[child] = xcb_get_property(c, false, children[child], WM_STATE, WM_STATE, 0, 2);
    }
    for (child = 0; child < child_count; child++)
    {
        window_attributes_replies[child] = xcb_get_window_attributes_reply(c, window_attributes_cookies[child], NULL);
        property_replies[child] = xcb_get_property_reply(c, property_cookies[child], NULL);
        state_replies[child] = xcb_get_property_reply(c, state_cookies[child], NULL);

        if (window_attributes_replies[child]->override_redirect || *(xcb_window_t *) xcb_get_property_value(property_replies[child]))
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
        if (*(xcb_window_t *) xcb_get_property_value(property_replies[child]) && (window_attributes_replies[child]->map_state == XCB_MAP_STATE_VIEWABLE || ((uint32_t *) xcb_get_property_value(state_replies[child]))[0] == XCB_WM_STATE_ICONIC))
        {
            manage(children[child]);
        }

        free(window_attributes_replies[child]);
        free(property_replies[child]);
    }
}

void spawn(const char ** command)
{
    if (fork() == 0)
    {
        if (c)
        {
            close(xcb_get_file_descriptor(c));
        }

        setsid();
        printf("executing\n");
        execvp(command[0], command);
        exit(0);
    }
}

void spawn_terminal()
{
    printf("spawning terminal\n");
    const char * command[] = { "urxvt", NULL };
    spawn(command);
}

void spawn_dmenu()
{
    printf("spawning dmenu\n");
    const char * command[] = {
        "dmenu_run",
        "-b",
        "-fn", "-*-terminus-medium-*-*-*-12-*-*-*-*-*-*-*",
        "-nb", "#222222",
        "-nf", "#999999",
        "-sb", "#338833",
        "-sf", "#FFFFFF",
        NULL
    };
    spawn(command);
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

    window = window_list_lookup(visible_windows, event->window);

    if (window)
    {
        /* Case 3 of the ICCCM 4.1.5 */
        if (event->value_mask & (XCB_CONFIG_WINDOW_BORDER_WIDTH | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT))
        {
            printf("configure_request: case 3\n");
            window->border_width = event->border_width;
            // TODO: Make sure this is right
        }
        /* Case 1 of the ICCCM 4.1.5 */
        else
        {
            printf("configure_request: case 1\n");
            synthetic_configure(window);
        }
    }
    /* Case 1 of the ICCCM 4.1.5 */
    else if (window_list_lookup(hidden_windows, event->window) != NULL) // Will this ever happen?
    {
        printf("configure_request: case 1\n");
        synthetic_configure(window);
    }
    /* Case 2 of the ICCCM 4.1.5 */
    else
    {
        uint16_t mask = 0;
        uint32_t values[7];
        uint8_t field = 0;

        printf("configure_request: case 2\n");

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

    xcb_flush(c);
}

void configure_notify(xcb_configure_notify_event_t * event)
{
    printf("configure_notify\n");

    printf("window_id: %i\n", event->window);

    if (event->window == root)
    {
        printf("!!!!!window is root\n");
        screen_width = event->width;
        screen_height = event->height;
    }
}

void destroy_notify(xcb_destroy_notify_event_t * event)
{
    struct mwm_window * window;

    printf("destroy_notify\n");

    printf("window_id: %i\n", event->window);

    window = window_list_lookup(visible_windows, event->window);
    if (window == NULL)
    {
        window = window_list_lookup(hidden_windows, event->window);
    }

    if (window != NULL)
    {
        unmanage(window);
    }
}

void enter_notify(xcb_enter_notify_event_t * event)
{
    struct mwm_window * window;

    printf("enter_notify\n");

    printf("window_id: %i\n", event->event);

    if (event->event == root)
    {
        focus(root);
    }
    else
    {
        window = window_list_lookup(visible_windows, event->event);

        if (window != NULL)
        {
            printf("mode: %i\n", event->mode);
            printf("detail: %i\n", event->detail);

            if (event->mode == XCB_NOTIFY_MODE_NORMAL && event->detail != XCB_NOTIFY_DETAIL_INFERIOR)
            {
                focus(window->window_id);
            }
        }
    }
}

void leave_notify(xcb_leave_notify_event_t * event)
{
    printf("leave_notify\n");
}

void key_press(xcb_key_press_event_t * event)
{
    xcb_keysym_t keysym = 0;
    struct mwm_key_binding_list * current_element;

    keysym = xcb_get_keyboard_mapping_keysyms(keyboard_mapping)[keyboard_mapping->keysyms_per_keycode * (event->detail - xcb_get_setup(c)->min_keycode)];

    printf("keysym: %i\n", keysym);
    printf("modifiers: %i\n", event->state);

    for (current_element = key_bindings; current_element != NULL; current_element = current_element->next)
    {
        if (keysym == current_element->binding.keysym && CLEAN_MASK(event->state) == current_element->binding.modifiers)
        {
            if (current_element->binding.function != NULL)
            {
                current_element->binding.function();
            }
        }
    }
}

void mapping_notify(xcb_mapping_notify_event_t * event)
{
    printf("mapping_notify: %i\n", event->request);

    if (event->request == XCB_MAPPING_KEYBOARD)
    {
        grab_keys(event->first_keycode, event->first_keycode + event->count - 1);
    }
}

void map_request(xcb_map_request_event_t * event)
{
    struct mwm_window * maybe_window;
    xcb_get_window_attributes_cookie_t window_attributes_cookie;
    xcb_get_window_attributes_reply_t * window_attributes;

    printf("map_request: id:%i\n", event->window);

    window_attributes_cookie = xcb_get_window_attributes(c, event->window);

    maybe_window = window_list_lookup(hidden_windows, event->window);

    window_attributes = xcb_get_window_attributes_reply(c, window_attributes_cookie, NULL);

    if (!maybe_window && window_attributes && !window_attributes->override_redirect)
    {
        manage(event->window);
    }
}

void property_notify(xcb_property_notify_event_t * event)
{
    printf("property_notify\n");
}

void unmap_notify(xcb_unmap_notify_event_t * event)
{
    struct mwm_window * window;

    printf("unmap_notify: %i\n", event->window);

    if (pending_unmaps > 0)
    {
        /* If we are expecting an unmap due to hiding a window, ignore it */
        pending_unmaps--;
        return;
    }

    window = window_list_lookup(visible_windows, event->window);

    if (window != NULL)
    {
        uint32_t property_values[2];

        printf("setting state to withdrawn\n");

        xcb_grab_server(c);

        property_values[0] = XCB_WM_STATE_WITHDRAWN;
        property_values[1] = 0;
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id, WM_STATE, WM_STATE, 32, 2, property_values);

        unmanage(window);

        xcb_flush(c);

        xcb_ungrab_server(c);
    }
}

void handle_event(xcb_generic_event_t * event)
{
    switch (event->response_type & ~0x80)
    {
        case XCB_BUTTON_PRESS:
            button_press((xcb_button_press_event_t *) event);
            break;
        case XCB_CONFIGURE_REQUEST:
            configure_request((xcb_configure_request_event_t *) event);
            break;
        case XCB_CONFIGURE_NOTIFY:
            configure_notify((xcb_configure_notify_event_t *) event);
            break;
        case XCB_DESTROY_NOTIFY:
            destroy_notify((xcb_destroy_notify_event_t *) event);
            break;
        case XCB_ENTER_NOTIFY:
            enter_notify((xcb_enter_notify_event_t *) event);
            break;
        case XCB_LEAVE_NOTIFY:
            leave_notify((xcb_leave_notify_event_t *) event);
            break;
        case XCB_KEY_PRESS:
            key_press((xcb_key_press_event_t *) event);
            break;
        case XCB_MAPPING_NOTIFY:
            mapping_notify((xcb_mapping_notify_event_t *) event);
            break;
        case XCB_MAP_REQUEST:
            map_request((xcb_map_request_event_t *) event);
            break;
        case XCB_PROPERTY_NOTIFY:
            property_notify((xcb_property_notify_event_t *) event);
            break;
        case XCB_UNMAP_NOTIFY:
            unmap_notify((xcb_unmap_notify_event_t *) event);
            break;

        default:
            printf("unhandled event type: %i\n", event->response_type & ~0x80);
            break;
    }

    free(event);
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
    cleanup_key_bindings();
    cleanup_tags();
    cleanup_layouts();

    /* X cursors */
    xcb_free_cursor(c, cursors[POINTER]);
    xcb_free_cursor(c, cursors[RESIZE]);
    xcb_free_cursor(c, cursors[MOVE]);

    // TODO: Free colors

    xcb_disconnect(c);
}

int main(int argc, char ** argv)
{
    srand(time(NULL));

    setup();
    manage_existing_windows();
    run_startup_hooks();
    run();
    cleanup();

    return 0;
}

