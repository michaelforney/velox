/* velox: velox/event_handler.c
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
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_icccm.h>

#include "event_handler.h"
#include "velox.h"
#include "window.h"
#include "hook.h"
#include "modifier.h"
#include "debug.h"
#include "list.h"
#include "keyboard_mapping.h"

#include "velox-private.h"
#include "hook-private.h"
#include "ewmh-private.h"
#include "binding-private.h"

#define DO(type, name)                                                      \
LIST_HEAD(name ## _event_handlers);                                         \
                                                                            \
struct name ## _event_handler_entry                                         \
{                                                                           \
    name ## _event_handler_t handler;                                       \
    struct list_head head;                                                  \
};                                                                          \
                                                                            \
void add_ ## name ## _event_handler(name ## _event_handler_t handler)       \
{                                                                           \
    struct name ## _event_handler_entry * entry =                           \
        malloc(sizeof(struct name ## _event_handler_entry));                \
    entry->handler = handler;                                               \
    list_add_tail(&entry->head, &name ## _event_handlers);                  \
}
#include "event_types.h"
#undef DO

void handle_event(xcb_generic_event_t * event)
{
    switch (event->response_type & ~0x80)
    {
#define DO(type, name)                                                      \
        case type:                                                          \
        {                                                                   \
            struct name ## _event_handler_entry * entry;                    \
                                                                            \
            list_for_each_entry(entry, &name ## _event_handlers, head)      \
            {                                                               \
                entry->handler((xcb_ ## name ## _event_t *) event);         \
            }                                                               \
                                                                            \
            break;                                                          \
        }
#include "event_types.h"
#undef DO
    }
}

/* X event handlers */
static void key_press(xcb_key_press_event_t * event)
{
    xcb_keysym_t keysym = XCB_NO_SYMBOL;
    struct velox_binding * binding;

    keysym = xcb_key_press_lookup_keysym(keyboard_mapping, event, 0);

    DEBUG_PRINT("keysym: %x\n", keysym)
    DEBUG_PRINT("modifiers: %i\n", event->state)

    vector_for_each(&key_bindings, binding)
    {
        if (keysym == binding->bindable.pressable.key &&
            ((binding->bindable.modifiers == XCB_MOD_MASK_ANY) ||
            (CLEAN_MASK(event->state) == binding->bindable.modifiers)))
        {
            if (binding->function != NULL)
            {
                binding->function(binding->arg);
            }
        }
    }
}

static void button_press(xcb_button_press_event_t * event)
{
    struct velox_binding * binding;
    xcb_button_t button;

    DEBUG_ENTER

    button = event->detail;

    DEBUG_PRINT("button: %u\n", button);
    DEBUG_PRINT("window: 0x%x\n", event->event);

    /* Mouse bindings are grabbed with the root window, so if the event window
     * is root, call any binding functions on the sub window */
    if (event->event == screen->root)
    {
        vector_for_each(&button_bindings, binding)
        {
            DEBUG_PRINT("binding button: %u\n", binding->bindable.pressable.button);
            if (button == binding->bindable.pressable.button &&
                ((binding->bindable.modifiers == XCB_MOD_MASK_ANY) ||
                (CLEAN_MASK(event->state) == binding->bindable.modifiers)))
            {
                if (binding->function != NULL)
                {
                    binding->function(window_id_argument(event->child));
                }
            }
        }
    }
    /* Otherwise, we are just clicking on the window to focus it */
    else
    {
        struct velox_window * window = lookup_window(event->event);

        focus(event->event);

        if (window && window->floating)
        {
            workspace->focus_type = FLOAT;
        }
        else
        {
            workspace->focus_type = TILE;
        }
    }
}

static void enter_notify(xcb_enter_notify_event_t * event)
{
    DEBUG_ENTER
    DEBUG_PRINT("window_id: 0x%x\n", event->event)

    if (workspace->focus_type == FLOAT) return;

    if (event->event == screen->root) focus(screen->root);
    else
    {
        struct velox_window_entry * entry;
        struct velox_window * window;

        window = NULL;

        /* Look through tiled windows */
        list_for_each_entry(entry, &workspace->tiled.windows, head)
        {
            if (entry->window->window_id == event->event)
            {
                window = entry->window;
                window->workspace->tiled.focus = &entry->head;
                break;
            }
        }

        if (window != NULL)
        {
            DEBUG_PRINT("mode: %i\n", event->mode)
            DEBUG_PRINT("detail: %i\n", event->detail)

            if (event->mode == XCB_NOTIFY_MODE_NORMAL && event->detail != XCB_NOTIFY_DETAIL_INFERIOR)
            {
                focus(window->window_id);
            }
        }
    }
}

static void leave_notify(xcb_leave_notify_event_t * event)
{
    DEBUG_ENTER
}

static void destroy_notify(xcb_destroy_notify_event_t * event)
{
    struct velox_window_entry * entry;

    DEBUG_ENTER
    DEBUG_PRINT("window_id: 0x%x\n", event->event)

    if (event->event == screen->root) return;

    unmanage(event->event);
}

static void unmap_notify(xcb_unmap_notify_event_t * event)
{
    struct velox_window_entry * entry;
    struct velox_window * window;

    DEBUG_ENTER
    DEBUG_PRINT("window_id: 0x%x\n", event->event);

    if (event->event == screen->root) return;

    uint32_t property_values[2];

    DEBUG_PRINT("setting state to withdrawn\n")

    xcb_grab_server(c);

    property_values[0] = XCB_ICCCM_WM_STATE_WITHDRAWN;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, event->window, WM_STATE, WM_STATE, 32, 2, property_values);

    unmanage(event->event);

    xcb_flush(c);

    xcb_ungrab_server(c);
}

static void map_request(xcb_map_request_event_t * event)
{
    struct velox_window * window;
    xcb_get_window_attributes_cookie_t window_attributes_cookie;
    xcb_get_window_attributes_reply_t * window_attributes;

    DEBUG_ENTER

    window_attributes_cookie = xcb_get_window_attributes(c, event->window);

    window = lookup_window(event->window);

    window_attributes = xcb_get_window_attributes_reply(c, window_attributes_cookie, NULL);

    if (window_attributes)
    {
        if (window == NULL && !window_attributes->override_redirect)
        {
            manage(event->window);
        }
        else
        {
            DEBUG_PRINT("not managing window with id: 0x%x\n", event->window);
        }

        free(window_attributes);
    }
}

static void configure_notify(xcb_configure_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->window == screen->root)
    {
        screen_area.width = event->width;
        screen_area.height = event->height;

        arrange();

        run_hooks(NULL, VELOX_HOOK_ROOT_RESIZED);
    }
}

static void configure_request(xcb_configure_request_event_t * event)
{
    struct velox_window * window;

    DEBUG_ENTER

    window = lookup_window(event->window);

    /* Case 1 of the ICCCM 4.1.5 */
    if (window != NULL && !window->floating)
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

static void property_notify(xcb_property_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->atom == XCB_ATOM_WM_NAME && event->state == XCB_PROPERTY_NEW_VALUE)
    {
        struct velox_window * window = lookup_window(event->window);

        if (window)
        {
            update_name_class(window);
            run_hooks(window, VELOX_HOOK_WINDOW_NAME_CHANGED);
        }
    }
    else
    {
        xcb_get_atom_name_cookie_t atom_name_cookie = xcb_get_atom_name(c, event->atom);
        xcb_get_atom_name_reply_t * atom_name_reply = xcb_get_atom_name_reply(c,
            atom_name_cookie, NULL);

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
}

static void client_message(xcb_client_message_event_t * event)
{
}

static void mapping_notify(xcb_mapping_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->request == XCB_MAPPING_KEYBOARD)
    {
        xcb_refresh_keyboard_mapping(keyboard_mapping, event);

        run_hooks(NULL, VELOX_HOOK_KEYBOARD_MAPPING_CHANGED);
    }
}

void setup_event_handlers()
{
    add_key_press_event_handler(&key_press);
    add_button_press_event_handler(&button_press);
    add_enter_notify_event_handler(&enter_notify);
    add_leave_notify_event_handler(&leave_notify);
    add_destroy_notify_event_handler(&destroy_notify);
    add_unmap_notify_event_handler(&unmap_notify);
    add_map_request_event_handler(&map_request);
    add_configure_notify_event_handler(&configure_notify);
    add_configure_request_event_handler(&configure_request);
    add_property_notify_event_handler(&property_notify);
    add_client_message_event_handler(&client_message);
    add_mapping_notify_event_handler(&mapping_notify);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

