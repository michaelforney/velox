/* velox: screen.c
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

#include "screen.h"
#include "layout.h"
#include "util.h"
#include "velox.h"
#include "window.h"
#include "protocol/velox-server-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <swc.h>

static struct layout * (* default_layouts[])() = {
    &tall_layout_new,
    &grid_layout_new,
};

static void usable_geometry_changed(void * data)
{
    struct screen * screen = data;

    screen_arrange(screen);
}

static void entered(void * data)
{
    struct screen * screen = data;

    velox.active_screen = screen;
    window_focus(screen->focus);
}

static const struct swc_screen_handler screen_handler = {
    .usable_geometry_changed = &usable_geometry_changed,
    .entered = &entered,
};

static void add_window(struct screen * screen, struct window * window)
{
    /* Remove the window from hidden window list and add it to the screen's list
     * of windows. */
    wl_list_remove(&window->link);
    wl_list_insert(screen->windows.prev, &window->link);
    ++screen->num_windows;
}

static void remove_window(struct screen * screen, struct window * window)
{
    /* Remove the window from the screen's list of windows, and add it to the
     * list of hidden windows. */
    wl_list_remove(&window->link);
    wl_list_insert(velox.hidden_windows.prev, &window->link);
    --screen->num_windows;
}

static void send_focus(struct screen * screen, struct wl_resource * resource)
{
    velox_screen_send_focus(resource, screen->focus ? screen->focus->swc->title
                                                    : NULL);
}

struct screen * screen_new(struct swc_screen * swc)
{
    struct screen * screen;
    struct tag * tag;
    struct layout * layout, * tmp;
    unsigned index;

    if (!(screen = malloc(sizeof *screen)))
        goto error0;

    wl_list_init(&screen->layouts);
    for (index = 0; index < ARRAY_LENGTH(default_layouts); ++index)
    {
        if (!(layout = default_layouts[index]()))
            goto error1;
        wl_list_insert(screen->layouts.prev, &layout->link);
    }
    screen->layout = wl_container_of(screen->layouts.next, layout, link);

    wl_list_init(&screen->tags);
    screen->mask = 0;
    if ((tag = find_unused_tag()))
        tag_set(tag, screen);
    screen->last_mask = screen->mask;

    wl_list_init(&screen->windows);
    screen->num_windows = 0;
    screen->focus = NULL;

    screen->swc = swc;
    wl_list_init(&screen->resources);
    swc_screen_set_handler(swc, &screen_handler, screen);

    return screen;

  error1:
    wl_list_for_each_safe(layout, tmp, &screen->layouts, link)
        free(layout);
  error0:
    return NULL;
}

void screen_arrange(struct screen * screen)
{
    struct window * window;

    layout_begin(screen->layout, &screen->swc->usable_geometry,
                 screen->num_windows);
    wl_list_for_each(window, &screen->windows, link)
        layout_arrange(screen->layout, window);
}

void screen_add_windows(struct screen * screen)
{
    struct window * window, * tmp;

    wl_list_for_each_safe(window, tmp, &velox.hidden_windows, link)
    {
        if (screen->mask & window->tag->mask)
        {
            add_window(screen, window);

            if (!screen->focus)
                screen_set_focus(screen, window);
        }
    }
}

void screen_remove_windows(struct screen * screen)
{
    struct window * window, * tmp;

    /* If we will be removing the focus, try to find a new focus nearby the old
     * one. */
    if (screen->focus && !(screen->focus->tag
                           && screen->mask & screen->focus->tag->mask))
    {
        struct wl_list * forward = screen->focus->link.next,
                       * backward = screen->focus->link.prev;

        while (true)
        {
            if (forward != &screen->windows)
            {
                window = wl_container_of(forward, window, link);

                if (screen->mask & window->tag->mask)
                {
                    screen_set_focus(screen, window);
                    break;
                }

                forward = forward->next;
            }

            if (backward != &screen->windows)
            {
                window = wl_container_of(backward, window, link);

                if (screen->mask & window->tag->mask)
                {
                    screen_set_focus(screen, window);
                    break;
                }

                backward = backward->next;
            }

            /* If there is no suitable candidate for focus, we can just remove
             * all the windows and set the focus to NULL. */
            screen_set_focus(screen, NULL);
            wl_list_insert_list(&velox.hidden_windows, &screen->windows);
            wl_list_init(&screen->windows);
            screen->num_windows = 0;
            return;
        }
    }

    wl_list_for_each_safe(window, tmp, &screen->windows, link)
    {
        if (!(window->tag && screen->mask & window->tag->mask))
            remove_window(screen, window);
    }
}

void screen_focus_next(struct screen * screen)
{
    struct wl_list * link;

    if (!screen->focus)
        return;

    link = screen->focus->link.next;

    if (link == &screen->windows)
        link = link->next;

    screen_set_focus(screen, wl_container_of(link, screen->focus, link));
}

void screen_focus_prev(struct screen * screen)
{
    struct wl_list * link;

    if (!screen->focus)
        return;

    link = screen->focus->link.prev;

    if (link == &screen->windows)
        link = link->prev;

    screen_set_focus(screen, wl_container_of(link, screen->focus, link));
}

void screen_set_focus(struct screen * screen, struct window * window)
{
    struct wl_resource * resource;

    if (window)
        assert(window->tag->screen == screen);

    screen->focus = window;

    wl_resource_for_each(resource, &screen->resources)
        send_focus(screen, resource);

    if (screen == velox.active_screen)
        window_focus(screen->focus);
}

void screen_set_tags(struct screen * screen, uint32_t mask)
{
    struct screen * original_screen;
    struct tag * tag, * unused_tag;
    uint32_t added = (screen->mask | mask) & ~screen->mask,
             removed = (screen->mask | mask) & ~mask;

    if (screen->mask == mask)
        return;

    while ((tag = next_tag(&removed)))
        tag_set(tag, NULL);
    screen_remove_windows(screen);

    /* Now, remove all the tags we want to add to this screen from their current
     * screens. */
    removed = added;
    while ((tag = next_tag(&removed)))
    {
        original_screen = tag->screen;
        tag_remove(tag, original_screen);

        if (original_screen)
        {
            /* Make sure screens always have a tag visible, if possible. */
            if (!original_screen->mask && (unused_tag = find_unused_tag()))
                tag_set(unused_tag, original_screen);
            screen_remove_windows(original_screen);
            screen_add_windows(original_screen);
        }
    }

    while ((tag = next_tag(&added)))
        tag_add(tag, screen);
    screen_add_windows(screen);
}

struct wl_resource * screen_bind(struct screen * screen,
                                 struct wl_client * client, uint32_t id)
{
    struct wl_resource * resource;
    struct tag * tag;

    resource = wl_resource_create(client, &velox_screen_interface, 1, id);

    if (!resource)
        return NULL;

    wl_list_insert(&screen->resources, wl_resource_get_link(resource));
    wl_resource_set_destructor(resource, &remove_resource);
    send_focus(screen, resource);

    wl_list_for_each(tag, &screen->tags, link)
        tag_send_screen(tag, client, NULL, resource);

    return resource;
}

void screen_focus_title_notify(struct screen * screen)
{
    struct wl_resource * resource;

    wl_resource_for_each(resource, &screen->resources)
        send_focus(screen, resource);
}

