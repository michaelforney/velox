/* velox: velox/event.c
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
#include <stdio.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

#include "velox.h"
#include "window.h"
#include "keybinding.h"
#include "debug.h"

#include "velox-private.h"
#include "keybinding-private.h"

/* Macros */
#define CLEAN_MASK(mask) (mask & ~(mod_mask_numlock | XCB_MOD_MASK_LOCK))

/* X event handlers */
static void button_press(xcb_button_press_event_t * event)
{
    DEBUG_ENTER
}

static void configure_request(xcb_configure_request_event_t * event)
{
    DEBUG_ENTER

    struct velox_window * window = NULL;

    window = tags_lookup_window(event->window);

    /* Case 1 of the ICCCM 4.1.5 */
    if (window && !window->floating)
    {
        DEBUG_PRINT("configure_request: case 1\n")
        synthetic_configure(window);
    }
    /* Case 2 of the ICCCM 4.1.5 */
    else
    {
        uint16_t mask = 0;
        uint32_t values[7];
        uint8_t field = 0;

        DEBUG_PRINT("configure_request: case 2\n")

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

static void configure_notify(xcb_configure_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->window == root)
    {
        screen_area.width = event->width;
        screen_area.height = event->height;
    }
}

static void destroy_notify(xcb_destroy_notify_event_t * event)
{
    struct velox_window * window;

    DEBUG_ENTER
    DEBUG_PRINT("window_id: %i\n", event->window)

    window = tags_lookup_window(event->window);

    if (window != NULL)
    {
        unmanage(window);
    }
}

static void enter_notify(xcb_enter_notify_event_t * event)
{
    DEBUG_ENTER
    DEBUG_PRINT("window_id: %i\n", event->event)

    if (event->event == root) focus(root);
    else
    {
        struct velox_loop * element;

        element = window_loop_locate(tag->windows, event->event);

        if (element != NULL)
        {
            struct velox_window * window;

            window = (struct velox_window *) element->data;
            DEBUG_PRINT("mode: %i\n", event->mode)
            DEBUG_PRINT("detail: %i\n", event->detail)

            if (event->mode == XCB_NOTIFY_MODE_NORMAL && event->detail != XCB_NOTIFY_DETAIL_INFERIOR)
            {
                focus(window->window_id);
                window->tag->focus = element;
            }
        }
    }
}

static void leave_notify(xcb_leave_notify_event_t * event)
{
    DEBUG_ENTER
}

static void key_press(xcb_key_press_event_t * event)
{
    xcb_keysym_t keysym = 0;
    struct velox_list * iterator;
    struct velox_key_binding * binding;

    keysym = xcb_get_keyboard_mapping_keysyms(keyboard_mapping)[keyboard_mapping->keysyms_per_keycode * (event->detail - xcb_get_setup(c)->min_keycode)];

    DEBUG_PRINT("keysym: %x\n", keysym)
    DEBUG_PRINT("modifiers: %i\n", event->state)

    for (iterator = key_bindings; iterator != NULL; iterator = iterator->next)
    {
        binding = (struct velox_key_binding *) iterator->data;

        if (keysym == binding->key->keysym &&
            ((binding->key->modifiers == XCB_MOD_MASK_ANY) ||
            (CLEAN_MASK(event->state) == binding->key->modifiers)))
        {
            if (binding->function != NULL)
            {
                binding->function(binding->arg);
            }
        }
    }
}

static void mapping_notify(xcb_mapping_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->request == XCB_MAPPING_KEYBOARD)
    {
        grab_keys(event->first_keycode, event->first_keycode + event->count - 1);
    }
}

static void map_request(xcb_map_request_event_t * event)
{
    struct velox_window * maybe_window;
    xcb_get_window_attributes_cookie_t window_attributes_cookie;
    xcb_get_window_attributes_reply_t * window_attributes;

    DEBUG_ENTER

    window_attributes_cookie = xcb_get_window_attributes(c, event->window);

    maybe_window = tags_lookup_window(event->window);

    window_attributes = xcb_get_window_attributes_reply(c, window_attributes_cookie, NULL);

    if (window_attributes)
    {
        if (!maybe_window && !window_attributes->override_redirect)
        {
            manage(event->window);
        }

        free(window_attributes);
    }
}

static void property_notify(xcb_property_notify_event_t * event)
{
    xcb_get_atom_name_cookie_t atom_name_cookie;
    xcb_get_atom_name_reply_t * atom_name_reply;

    DEBUG_ENTER

    atom_name_cookie = xcb_get_atom_name(c, event->atom);
    atom_name_reply = xcb_get_atom_name_reply(c, atom_name_cookie, NULL);

    if (atom_name_reply)
    {
        char * atom_name = strndup(
            xcb_get_atom_name_name(atom_name_reply),
            xcb_get_atom_name_name_length(atom_name_reply)
        );
        DEBUG_PRINT("atom: %s\n", atom_name)
        free(atom_name);
    }

    free(atom_name_reply);
}

static void unmap_notify(xcb_unmap_notify_event_t * event)
{
    struct velox_window * window;

    DEBUG_ENTER

    if (pending_unmaps > 0)
    {
        /* If we are expecting an unmap due to hiding a window, ignore it */
        pending_unmaps--;
        return;
    }

    window = window_loop_lookup(tag->windows, event->window);

    if (window != NULL)
    {
        uint32_t property_values[2];

        DEBUG_PRINT("setting state to withdrawn\n")

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
            DEBUG_PRINT("unhandled event type: %i\n", event->response_type & ~0x80)
            break;
    }

    free(event);
}

