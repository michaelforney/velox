/* velox: velox/bound_layer.c
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

#include "layer.h"
#include "layout.h"
#include "window.h"
#include "work_area.h"
#include "velox.h"
#include "debug.h"

#include "velox-private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct bound_layer
{
    struct velox_layer base;

    struct velox_list layouts;
    struct velox_layout_entry * layout_entry;
    struct velox_layout_state layout_state;
};

_Static_assert(offsetof(struct bound_layer, base) == 0,
               "Non-zero offset of base field");

static void bound_layer_update(struct velox_layer * layer);
static void bound_layer_destroy(struct velox_layer * layer);
static void bound_layer_focus_next(struct velox_layer * layer);
static void bound_layer_focus_prev(struct velox_layer * layer);
static void bound_layer_swap_next(struct velox_layer * layer);
static void bound_layer_swap_prev(struct velox_layer * layer);
static void bound_layer_add_window(struct velox_layer * layer,
                            struct velox_window * window);
static void bound_layer_remove_window(struct velox_layer * layer,
                                      struct velox_window * window);
static void bound_layer_set_focus(struct velox_layer * layer,
                                  struct velox_window * window);

const struct velox_layer_interface bound_layer_interface = {
    .identifier = "bound",
    .update = &bound_layer_update,
    .destroy = &bound_layer_destroy,
    .focus_next = &bound_layer_focus_next,
    .focus_prev = &bound_layer_focus_prev,
    .swap_next = &bound_layer_swap_next,
    .swap_prev = &bound_layer_swap_prev,
    .add_window = &bound_layer_add_window,
    .remove_window = &bound_layer_remove_window,
    .set_focus = &bound_layer_set_focus
};

struct velox_layer * bound_layer_new(const char * layout_names[],
                                     uint32_t num_layout_names)
{
    struct bound_layer * layer;
    struct velox_layout_entry * entry;
    uint32_t index;

    if (num_layout_names == 0)
    {
        fprintf(stderr, "bound layer must have at least one layout\n");
        goto error0;
    }

    layer = malloc(sizeof *layer);

    if (!layer)
        goto error0;

    layer->base.focus = NULL;
    layer->base.interface = &bound_layer_interface;
    list_init(&layer->base.windows);
    list_init(&layer->layouts);

    for (index = 0; index < num_layout_names; ++index)
    {
        entry = malloc(sizeof(*entry));

        if (!entry || !(entry->layout = find_layout(layout_names[index])))
            goto error1;

        list_append(&layer->layouts, entry);
    }

    entry = list_first(&layer->layouts, typeof(*entry));
    memcpy(&layer->layout_state, entry->layout->default_state,
           entry->layout->default_state_size);
    layer->layout_entry = entry;

    return &layer->base;

  error1:
    free(layer);
  error0:
    return NULL;
}

struct velox_layout * bound_layer_get_layout(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;

    return layer->layout_entry->layout;
}

struct velox_layout_state * bound_layer_get_layout_state
    (struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;

    return &layer->layout_state;
}

static void set_layout(struct bound_layer * layer,
                       struct velox_layout_entry * layout_entry)
{
    struct velox_layout * layout;

    layer->layout_entry = layout_entry;
    memcpy(&layer->layout_state, layout_entry->layout->default_state,
           layout_entry->layout->default_state_size);

    layer_update(&layer->base);
}

void bound_layer_next_layout(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;

    set_layout(layer, list_next(&layer->layouts, layer->layout_entry));
}

void bound_layer_prev_layout(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;

    set_layout(layer, list_prev(&layer->layouts, layer->layout_entry));
}

bool layer_is_bound(struct velox_layer * layer)
{
    return layer->interface == &bound_layer_interface;
}

static void bound_layer_update(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;

    DEBUG_ENTER

    if (list_is_empty(&layer->base.windows)) return;

    calculate_work_area(&screen_area, &work_area);
    layer->layout_entry->layout->arrange(&work_area, &layer->base.windows,
                                         &layer->layout_state);
}

static void bound_layer_destroy(struct velox_layer * base)
{
    struct velox_window * window;
    struct velox_layout_entry * layout_entry;
    struct velox_link * tmp;
    struct bound_layer * layer = (void *) base;

    /* Free the layers's windows */
    list_for_each_entry_safe(&layer->base.windows, window, tmp)
        free(window);

    /* Free the layers's layout entries */
    list_for_each_entry_safe(&layer->layouts, layout_entry, tmp)
        free(layout_entry);

    free(layer);
}

static void bound_layer_focus_next(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;

    if (list_is_trivial(&layer->base.windows))
        return;

    layer->base.focus = list_next(&layer->base.windows, layer->base.focus);
}

static void bound_layer_focus_prev(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;

    if (list_is_trivial(&layer->base.windows))
        return;

    layer->base.focus = list_prev(&layer->base.windows, layer->base.focus);
}

static void bound_layer_swap_next(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;
    struct velox_window * next;

    if (list_is_trivial(&layer->base.windows))
        return;

    next = list_next(&layer->base.windows, layer->base.focus);
    link_swap(&layer->base.focus->link, &next->link);
}

static void bound_layer_swap_prev(struct velox_layer * base)
{
    struct bound_layer * layer = (void *) base;
    struct velox_window * prev;

    if (list_is_trivial(&layer->base.windows))
        return;

    prev = list_prev(&layer->base.windows, layer->base.focus);
    link_swap(&layer->base.focus->link, &prev->link);
}

static void bound_layer_add_window(struct velox_layer * base,
                                   struct velox_window * window)
{
    struct bound_layer * layer = (void *) base;

    list_append(&layer->base.windows, window);
    layer->base.focus = window;
}

static void bound_layer_remove_window(struct velox_layer * base,
                                      struct velox_window * window)
{
    struct bound_layer * layer = (void *) base;
    struct velox_window * next_focus = NULL;

    if (window == layer->base.focus && !list_is_trivial(&layer->base.windows))
        next_focus = list_next(&layer->base.windows, layer->base.focus);

    list_del(window);

    if (next_focus)
        layer->base.focus = next_focus;
}

static void bound_layer_set_focus(struct velox_layer * layer,
                                  struct velox_window * window)
{
    layer->focus = window;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

