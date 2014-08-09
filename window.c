/* velox: window.c
 *
 * Copyright (c) 2014 Michael Forney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "window.h"
#include "config.h"
#include "screen.h"
#include "tag.h"
#include "velox.h"

#include <stdlib.h>
#include <swc.h>

static uint32_t border_color_active = 0xff338833;
static uint32_t border_color_inactive = 0xff888888;

static bool border_width_set(struct config_node * node, const char * value)
{
    return config_set_unsigned(&border_width, value, 0);
}

static bool border_color_active_set(struct config_node * node,
                                    const char * value)
{
    return config_set_unsigned(&border_color_active, value, 16);
}

static bool border_color_inactive_set(struct config_node * node,
                                      const char * value)
{
    return config_set_unsigned(&border_color_inactive, value, 16);
}

static void begin_move(struct config_node * node)
{
    struct window * focus = velox.active_screen->focus;

    if (focus)
        swc_window_begin_move(focus->swc);
}

static void end_move(struct config_node * node)
{
    struct window * focus = velox.active_screen->focus;

    if (focus)
        swc_window_end_move(focus->swc);
}

static void begin_resize(struct config_node * node)
{
    struct window * focus = velox.active_screen->focus;

    if (focus)
        swc_window_begin_resize(focus->swc, SWC_WINDOW_EDGE_AUTO);
}

static void end_resize(struct config_node * node)
{
    struct window * focus = velox.active_screen->focus;

    if (focus)
        swc_window_end_resize(focus->swc);
}

static CONFIG_GROUP(window);
static CONFIG_PROPERTY(border_width, &border_width_set);
static CONFIG_PROPERTY(border_color_active, &border_color_active_set);
static CONFIG_PROPERTY(border_color_inactive, &border_color_inactive_set);
static CONFIG_ACTION(begin_move, &begin_move);
static CONFIG_ACTION(end_move, &end_move);
static CONFIG_ACTION(begin_resize, &begin_resize);
static CONFIG_ACTION(end_resize, &end_resize);

void window_add_config_nodes()
{
    wl_list_insert(&window_group.group, &border_width_property.link);
    wl_list_insert(&window_group.group, &border_color_active_property.link);
    wl_list_insert(&window_group.group, &border_color_inactive_property.link);
    wl_list_insert(&window_group.group, &begin_move_action.link);
    wl_list_insert(&window_group.group, &end_move_action.link);
    wl_list_insert(&window_group.group, &begin_resize_action.link);
    wl_list_insert(&window_group.group, &end_resize_action.link);
    wl_list_insert(config_root, &window_group.link);
}

static void window_event(struct wl_listener * listener, void * data)
{
    struct swc_event * event = data;
    struct window * window;

    window = wl_container_of(listener, window, event_listener);

    switch (event->type)
    {
        case SWC_WINDOW_DESTROYED:
            unmanage(window);
            wl_list_remove(&window->event_listener.link);
            free(window);
            break;
        case SWC_WINDOW_TITLE_CHANGED:
            /* If this window focused on a screen, make sure bound clients are
             * aware of this title change. */
            if (window->tag->screen && window->tag->screen->focus == window)
                screen_focus_title_notify(window->tag->screen);
            break;
        case SWC_WINDOW_ENTERED:
            window_focus(window);
            window->tag->screen->focus = window;
            break;
        case SWC_WINDOW_PARENT_CHANGED:
            /* TODO: Implement */
            break;
    }
}

struct window * window_new(struct swc_window * swc)
{
    struct window * window;

    if (!(window = malloc(sizeof *window)))
        return NULL;

    window->swc = swc;
    window->event_listener.notify = &window_event;
    window->tag = NULL;
    wl_signal_add(&swc->event_signal, &window->event_listener);

    return window;
}

void window_set_tag(struct window * window, struct tag * tag)
{
    struct tag * original_tag = window->tag;

    window->tag = tag;

    if (original_tag && original_tag->screen)
        screen_remove_windows(original_tag->screen);

    if (tag && tag->screen)
        screen_add_windows(tag->screen);
}

void window_focus(struct window * window)
{
    /* This will become stale if the focused window is destroyed. However, we
     * make sure to change the focus of a screen in screen_remove_windows when
     * its focus is removed (before it is actually destroyed). */
    static struct window * focused_window;

    if (focused_window)
    {
        swc_window_set_border(focused_window->swc,
                              border_color_inactive, border_width);
    }

    if (window)
    {
        swc_window_set_border(window->swc, border_color_active, border_width);
        swc_window_focus(window->swc);
    }
    else
        swc_window_focus(NULL);

    focused_window = window;
}

void window_show(struct window * window)
{
    swc_window_show(window->swc);
}

void window_hide(struct window * window)
{
    swc_window_hide(window->swc);
}

