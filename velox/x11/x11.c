/* velox: velox/x11.c
 *
 * Copyright (c) 2013 Michael Forney <mforney@mforney.org>
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

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_icccm.h>

#include "velox.h"
#include "hook.h"
#include "modifier.h"
#include "debug.h"

#include "x11/x11.h"
//#include "x11/modifier.h"
#include "x11/atom.h"
#include "x11/window.h"

#include "velox-private.h"
#include "binding-private.h"
#include "hook-private.h"

#include "x11/ewmh-private.h"
#include "x11/keyboard_mapping-private.h"
#include "x11/event_handler-private.h"

/* X constants */
const uint16_t border_color[] = { 0x9999, 0x9999, 0x9999 };
const uint16_t border_focus_color[] = { 0x3333,  0x8888, 0x3333 };

const uint32_t client_mask = XCB_EVENT_MASK_ENTER_WINDOW |
                             XCB_EVENT_MASK_PROPERTY_CHANGE |
                             XCB_EVENT_MASK_STRUCTURE_NOTIFY;

/* X variables */
xcb_connection_t * c;
xcb_screen_t * screen;
uint32_t border_pixel;
uint32_t border_focus_pixel;
uint8_t clear_event_type = 0;
int x11_fd;

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

void grab_buttons()
{
    struct velox_binding * binding;
    uint16_t index;
    uint16_t extra_modifiers_length = sizeof(extra_modifiers) / sizeof(uint16_t);

    xcb_ungrab_button(c, XCB_BUTTON_INDEX_ANY, screen->root, XCB_MOD_MASK_ANY);

    vector_for_each(&button_bindings, binding)
    {
        for (index = 0; index < extra_modifiers_length; ++index)
        {
            xcb_grab_button(c, false, screen->root,
                XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_SYNC,
                XCB_WINDOW_NONE, XCB_CURSOR_NONE,
                binding->bindable.pressable.button,
                binding->bindable.modifiers);
        }
    }

    xcb_flush(c);
}

static void check_wm_running()
{
    uint16_t mask;
    uint32_t values[1];
    xcb_void_cookie_t change_attributes_cookie;
    xcb_generic_error_t * error;

    mask = XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
    change_attributes_cookie = xcb_change_window_attributes_checked(c, screen->root, mask, values);

    error = xcb_request_check(c, change_attributes_cookie);
    if (error)
    {
        die("Another window manager is already running");
    }
}

struct velox_window * lookup_tiled_window(xcb_window_t window_id)
{
    struct velox_workspace * workspace_pointer;
    struct velox_window_entry * window_entry;

    vector_for_each(&workspaces, workspace_pointer)
    {
        list_for_each_entry(&workspace_pointer->tiled.windows, window_entry)
        {
            if (window_entry->window->window_id == window_id)
            {
                return window_entry->window;
            }
        }
    }

    return NULL;
}

struct velox_window * lookup_floated_window(xcb_window_t window_id)
{
    struct velox_workspace * workspace_pointer;
    struct velox_window_entry * window_entry;

    vector_for_each(&workspaces, workspace_pointer)
    {
        list_for_each_entry(&workspace_pointer->floated.windows, window_entry)
        {
            if (window_entry->window->window_id == window_id)
            {
                return window_entry->window;
            }
        }
    }

    return NULL;
}

struct velox_window * lookup_window(xcb_window_t window_id)
{
    struct velox_workspace * workspace_pointer;
    struct velox_window_entry * window_entry;

    vector_for_each(&workspaces, workspace_pointer)
    {
        list_for_each_entry(&workspace_pointer->tiled.windows, window_entry)
        {
            if (window_entry->window->window_id == window_id)
            {
                return window_entry->window;
            }
        }

        list_for_each_entry(&workspace_pointer->floated.windows, window_entry)
        {
            if (window_entry->window->window_id == window_id)
            {
                return window_entry->window;
            }
        }
    }

    return NULL;
}

void show_x11_window(struct velox_window * window)
{
    uint32_t property_values[2];

    DEBUG_ENTER

    property_values[0] = XCB_ICCCM_WM_STATE_NORMAL;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id,
        WM_STATE, WM_STATE, 32, 2, property_values);

    xcb_map_window(c, window->window_id);
}

void hide_x11_window(struct velox_window * window)
{
    uint32_t values[2];

    DEBUG_ENTER

    values[0] = XCB_ICCCM_WM_STATE_WITHDRAWN;
    values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id,
        WM_STATE, WM_STATE, 32, 2, values);

    /* Temporarily disable structure notify events so we don't get an unmap
     * event */
    values[0] = client_mask & ~XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    xcb_change_window_attributes(c, window->window_id, XCB_CW_EVENT_MASK, values);

    xcb_unmap_window(c, window->window_id);

    values[0] = client_mask;
    xcb_change_window_attributes(c, window->window_id, XCB_CW_EVENT_MASK, values);
}

void focus_x11_window(struct velox_window * window)
{
    uint16_t mask = XCB_CW_BORDER_PIXEL;
    uint32_t values[1];

    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    xcb_window_t window_id = window ? window->window_id : screen->root;

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    if (focus_reply->focus == window_id)
    {
        DEBUG_PRINT("already focused\n");
        free(focus_reply);
        return;
    }

    DEBUG_PRINT("window_id: 0x%x\n", window_id)

    if (window_id != screen->root)
    {
        DEBUG_PRINT("setting focused border\n");
        values[0] = border_focus_pixel;
        xcb_change_window_attributes(c, window_id, mask, values);

        xcb_ungrab_button(c, XCB_BUTTON_INDEX_ANY, window_id, XCB_MOD_MASK_ANY);
    }

    if (focus_reply->focus != screen->root)
    {
        xcb_button_t buttons[] = { 1, 2, 3 };
        uint8_t index;

        values[0] = border_pixel;
        xcb_change_window_attributes(c, focus_reply->focus, mask, values);

        for (index = 0; index < sizeof(buttons) / sizeof(xcb_button_t); ++index)
        {
            xcb_grab_button(c, false, focus_reply->focus,
                XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_SYNC,
                XCB_WINDOW_NONE, XCB_CURSOR_NONE,
                buttons[index], XCB_MOD_MASK_ANY);
        }
    }

    free(focus_reply);

    xcb_set_input_focus(c, XCB_INPUT_FOCUS_POINTER_ROOT, window_id, XCB_TIME_CURRENT_TIME);
    run_hooks(&window_id, VELOX_HOOK_FOCUS_CHANGED);

    xcb_flush(c);
}

void handle_x11_data()
{
    xcb_generic_event_t * event;

    while ((event = xcb_poll_for_event(c)))
    {
        handle_event(event);
        free(event);

        if (clear_event_type)
        {
            xcb_aux_sync(c);

            while ((event = xcb_poll_for_event(c)))
            {
                if ((event->response_type & ~0x80) == clear_event_type)
                {
                    DEBUG_PRINT("dropping masked event\n")
                }
                else
                {
                    handle_event(event);
                }

                free(event);
            }

            clear_event_type = 0;
        }
    }
}

void manage(xcb_window_t window_id)
{
    DEBUG_ENTER

    struct velox_window * window = NULL;
    struct velox_window_entry * window_entry;
    xcb_get_property_cookie_t transient_for_cookie;
    xcb_get_property_reply_t * transient_for_reply = NULL;
    xcb_get_geometry_cookie_t geometry_cookie;
    xcb_get_geometry_reply_t * geometry = NULL;
    xcb_window_t transient_id = 0;
    struct velox_window * transient = NULL;
    uint32_t mask;
    uint32_t values[2];
    uint32_t property_values[2];

    transient_for_cookie = xcb_icccm_get_wm_transient_for(c, window_id);
    geometry_cookie = xcb_get_geometry(c, window_id);

    window = (struct velox_window *) malloc(sizeof(struct velox_window));
    DEBUG_PRINT("allocated window: %p\n", window)
    DEBUG_PRINT("window_id: 0x%x\n", window_id);
    window->window_id = window_id;

    transient_for_reply = xcb_get_property_reply(c, transient_for_cookie, NULL);

    if (xcb_icccm_get_wm_transient_for_reply(c, transient_for_cookie, &transient_id, NULL))
    {
        DEBUG_PRINT("transient_id: %i\n", transient_id)
        transient = lookup_window(transient_id);

        window->floating = true;
    }
    else
    {
        window->floating = false;
    }

    if (transient != NULL)
    {
        window->workspace = transient->workspace;
    }
    else
    {
        window->workspace = workspace;
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
    values[1] = client_mask;

    xcb_change_window_attributes(c, window_id, mask, values);

    synthetic_configure(window);

    if (window->floating)
    {
        window_entry = (struct velox_window_entry *) malloc(sizeof(struct velox_window_entry));
        window_entry->window = window;

        list_append(&window->workspace->floated.windows, window_entry);

        window->workspace->focus_type = FLOAT;
    }
    else
    {
        window_entry = (struct velox_window_entry *) malloc(sizeof(struct velox_window_entry));
        window_entry->window = window;

        list_append(&window->workspace->tiled.windows, window_entry);

        window->workspace->tiled.focus =
            list_last_link(&window->workspace->tiled.windows);

        window->workspace->focus_type = TILE;
    }

    if (workspace == window->workspace)
    {
        if (window->floating) restack();
        else
        {
            arrange();
            restack();
        }

        xcb_map_window(c, window->window_id);

        property_values[0] = XCB_ICCCM_WM_STATE_NORMAL;
        property_values[1] = 0;
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->window_id,
            WM_STATE, WM_STATE, 32, 2, property_values);

        DEBUG_PRINT("focusing window: %u\n", window->window_id);
        focus(window);

        run_hooks(window, VELOX_HOOK_MANAGE_POST);
    }
}

void unmanage(xcb_window_t window_id)
{
    struct velox_workspace * workspace_pos;
    struct velox_window_entry * window_entry;
    struct velox_window * window = NULL;

    DEBUG_ENTER
    DEBUG_PRINT("window_id: 0x%x\n", window_id);

    vector_for_each(&workspaces, workspace_pos)
    {
        /* Look through the tiled windows */
        list_for_each_entry(&workspace_pos->tiled.windows, window_entry)
        {
            if (window_entry->window->window_id == window_id)
            {
                window = window_entry->window;

                /* Deal with special cases */
                if (&window_entry->link == workspace_pos->tiled.focus)
                {
                    workspace_pos->tiled.focus = list_next(&workspace_pos->tiled.windows,
                        workspace_pos->tiled.focus);
                }

                list_del(window_entry);
                free(window_entry);

                /* If there are no more tiled windows, but floated windows
                 * still exist, switch the focus type to FLOAT */
                if (list_is_empty(&workspace_pos->tiled.windows)
                    && !list_is_empty(&workspace_pos->floated.windows))
                {
                    workspace_pos->focus_type = FLOAT;
                }

                arrange();

                goto found;
            }
        }

        list_for_each_entry(&workspace_pos->floated.windows, window_entry)
        {
            if (window_entry->window->window_id == window_id)
            {
                window = window_entry->window;

                list_del(window_entry);
                free(window_entry);

                /* If there are no more floated windows, but tiled windows
                 * still exist, switch the focus type to TILE */
                if (list_is_empty(&workspace_pos->floated.windows)
                    && !list_is_empty(&workspace_pos->tiled.windows))
                {
                    workspace_pos->focus_type = TILE;
                }

                arrange();

                goto found;
            }
        }
    }

    return;

    found:
        DEBUG_PRINT("found\n")

        if (workspace == workspace_pos)
        {
            update_focus(workspace);
        }

        run_hooks(window, VELOX_HOOK_UNMANAGE);
        free(window);
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
    xcb_get_property_cookie_t * state_cookies;
    xcb_get_property_reply_t ** state_replies;

    query_cookie = xcb_query_tree(c, screen->root);
    query_reply = xcb_query_tree_reply(c, query_cookie, NULL);
    children = xcb_query_tree_children(query_reply);
    child_count = xcb_query_tree_children_length(query_reply);

    window_attributes_cookies = (xcb_get_window_attributes_cookie_t *)
        malloc(child_count * sizeof(xcb_get_window_attributes_cookie_t));
    window_attributes_replies = (xcb_get_window_attributes_reply_t **)
        malloc(child_count * sizeof(xcb_get_window_attributes_reply_t *));
    state_cookies = (xcb_get_property_cookie_t *)
        malloc(child_count * sizeof(xcb_get_property_cookie_t));
    state_replies = (xcb_get_property_reply_t **)
        malloc(child_count * sizeof(xcb_get_property_reply_t *));

    DEBUG_PRINT("child_count: %i\n", child_count)

    for (child = 0; child < child_count; ++child)
    {
        window_attributes_cookies[child] = xcb_get_window_attributes(c, children[child]);
        state_cookies[child] = xcb_get_property(c, false, children[child],
            WM_STATE, WM_STATE, 0, 2);
    }

    for (child = 0; child < child_count; ++child)
    {
        window_attributes_replies[child] = xcb_get_window_attributes_reply(c,
            window_attributes_cookies[child], NULL);
        state_replies[child] = xcb_get_property_reply(c, state_cookies[child], NULL);

        if (window_attributes_replies[child]->override_redirect)
        {
            DEBUG_PRINT("override_redirect\n")
            continue;
        }

        if (window_attributes_replies[child]->map_state == XCB_MAP_STATE_VIEWABLE ||
            ((uint32_t *) xcb_get_property_value(state_replies[child]))[0] == XCB_ICCCM_WM_STATE_ICONIC)
        {
            manage(children[child]);
        }

        free(window_attributes_replies[child]);
        free(state_replies[child]);
    }

    free(query_reply);
    free(window_attributes_cookies);
    free(state_cookies);
    free(window_attributes_replies);
    free(state_replies);
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

void setup_x11()
{
    const xcb_setup_t * setup;
    xcb_screen_iterator_t screen_iterator;
    xcb_font_t cursor_font;
    xcb_alloc_color_cookie_t border_color_cookie;
    xcb_alloc_color_cookie_t border_focus_color_cookie;
    xcb_alloc_color_reply_t * border_color_reply;
    xcb_alloc_color_reply_t * border_focus_color_reply;
    uint32_t mask;
    uint32_t values[2];

    c = xcb_connect(NULL, NULL);

    if (xcb_connection_has_error(c)) die("Could not open display");

    x11_fd = xcb_get_file_descriptor(c);

    setup = xcb_get_setup(c);

    screen_iterator = xcb_setup_roots_iterator(setup);

    screen = screen_iterator.data;
    screen_area.width = screen->width_in_pixels;
    screen_area.height = screen->height_in_pixels;

    work_area = screen_area;

    check_wm_running();

    /* Allocate colors */
    border_color_cookie = xcb_alloc_color(c, screen->default_colormap,
        border_color[0], border_color[1], border_color[2]);
    border_focus_color_cookie = xcb_alloc_color(c, screen->default_colormap,
        border_focus_color[0], border_focus_color[1], border_focus_color[2]);

    setup_atoms();

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
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    values[1] = cursors[POINTER];

    xcb_change_window_attributes(c, screen->root, mask, values);

    /* Check color allocation replies */
    border_color_reply = xcb_alloc_color_reply(c, border_color_cookie, NULL);
    border_focus_color_reply = xcb_alloc_color_reply(c, border_focus_color_cookie, NULL);

    border_pixel = border_color_reply->pixel;
    border_focus_pixel = border_focus_color_reply->pixel;

    free(border_color_reply);
    free(border_focus_color_reply);

    setup_ewmh();
    setup_keyboard_mapping();
    setup_event_handlers();
}

void cleanup_x11()
{
    cleanup_ewmh();

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
    }

    xcb_disconnect(c);
}

void kill_focused_window()
{
    xcb_get_input_focus_cookie_t focus_cookie;
    xcb_get_input_focus_reply_t * focus_reply;

    focus_cookie = xcb_get_input_focus(c);
    focus_reply = xcb_get_input_focus_reply(c, focus_cookie, NULL);

    DEBUG_ENTER

    if (focus_reply->focus == screen->root)
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

void move_float(union velox_argument argument)
{
    xcb_window_t window_id = argument.window_id;
    struct velox_window * window;
    xcb_grab_pointer_cookie_t grab_cookie;
    xcb_grab_pointer_reply_t * grab_reply;

    if ((window = lookup_floated_window(window_id)) == NULL) return;

    grab_cookie = xcb_grab_pointer(c, false, screen->root, XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_WINDOW_NONE,
        cursors[MOVE], XCB_CURRENT_TIME);

    grab_reply = xcb_grab_pointer_reply(c, grab_cookie, NULL);

    if (grab_reply->status == XCB_GRAB_STATUS_SUCCESS)
    {
        xcb_generic_event_t * event;
        uint32_t original_x, original_y;
        xcb_query_pointer_cookie_t pointer_cookie;
        xcb_query_pointer_reply_t * pointer_reply;

        pointer_cookie = xcb_query_pointer(c, screen->root);
        pointer_reply = xcb_query_pointer_reply(c, pointer_cookie, NULL);

        original_x = window->x;
        original_y = window->y;

        while ((event = xcb_wait_for_event(c)))
        {
            if ((event->response_type & ~0x80) == XCB_BUTTON_RELEASE)
            {
                free(event);
                break;
            }
            else if ((event->response_type & ~0x80) == XCB_MOTION_NOTIFY)
            {
                window->x = original_x +
                    ((xcb_motion_notify_event_t *) event)->event_x -
                    pointer_reply->root_x;

                window->y = original_y +
                    ((xcb_motion_notify_event_t *) event)->event_y -
                    pointer_reply->root_y;

                arrange_window(window);

                xcb_flush(c);
            }
            else
            {
                handle_event(event);
            }

            free(event);
        }


        free(pointer_reply);

        xcb_ungrab_pointer(c, XCB_CURRENT_TIME);
        xcb_flush(c);
    }

    free(grab_reply);
}

void resize_float(union velox_argument argument)
{
    xcb_window_t window_id = argument.window_id;
    struct velox_window * window;
    xcb_grab_pointer_cookie_t grab_cookie;
    xcb_grab_pointer_reply_t * grab_reply;

    if ((window = lookup_floated_window(window_id)) == NULL) return;

    grab_cookie = xcb_grab_pointer(c, false, screen->root, XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_WINDOW_NONE,
        cursors[RESIZE], XCB_CURRENT_TIME);

    grab_reply = xcb_grab_pointer_reply(c, grab_cookie, NULL);

    if (grab_reply->status == XCB_GRAB_STATUS_SUCCESS)
    {
        xcb_generic_event_t * event;
        uint32_t original_width, original_height;
        xcb_query_pointer_cookie_t pointer_cookie;
        xcb_query_pointer_reply_t * pointer_reply;

        pointer_cookie = xcb_query_pointer(c, screen->root);
        pointer_reply = xcb_query_pointer_reply(c, pointer_cookie, NULL);

        original_width = window->width;
        original_height = window->height;

        while ((event = xcb_wait_for_event(c)))
        {
            if ((event->response_type & ~0x80) == XCB_BUTTON_RELEASE)
            {
                free(event);
                break;
            }
            else if ((event->response_type & ~0x80) == XCB_MOTION_NOTIFY)
            {
                window->width = original_width +
                    ((xcb_motion_notify_event_t *) event)->event_x -
                    pointer_reply->root_x;

                window->height = original_height +
                    ((xcb_motion_notify_event_t *) event)->event_y -
                    pointer_reply->root_y;

                arrange_window(window);

                xcb_flush(c);
            }
            else
            {
                handle_event(event);
            }

            free(event);
        }


        free(pointer_reply);

        xcb_ungrab_pointer(c, XCB_CURRENT_TIME);
        xcb_flush(c);
    }

    free(grab_reply);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

