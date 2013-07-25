/* velox: velox/free_layer.c
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
#include "window.h"

#include "velox-private.h"

#include <stdlib.h>

static void free_layer_update(struct velox_layer * layer);
static void free_layer_destroy(struct velox_layer * layer);
static void free_layer_focus_next(struct velox_layer * layer);
static void free_layer_focus_prev(struct velox_layer * layer);
static void free_layer_swap_next(struct velox_layer * layer);
static void free_layer_swap_prev(struct velox_layer * layer);
static void free_layer_add_window(struct velox_layer * layer,
                                  struct velox_window * window);
static void free_layer_remove_window(struct velox_layer * layer,
                                     struct velox_window * window);
static void free_layer_set_focus(struct velox_layer * layer,
                                 struct velox_window * window);

const struct velox_layer_interface free_layer_interface = {
    .identifier = "free",
    .update = &free_layer_update,
    .destroy = &free_layer_destroy,
    .focus_next = &free_layer_focus_next,
    .focus_prev = &free_layer_focus_prev,
    .swap_next = &free_layer_swap_next,
    .swap_prev = &free_layer_swap_prev,
    .add_window = &free_layer_add_window,
    .remove_window = &free_layer_remove_window,
    .set_focus = &free_layer_set_focus
};

struct velox_layer * free_layer_new()
{
    struct velox_layer * layer;

    layer = malloc(sizeof *layer);

    if (!layer)
        return NULL;

    layer->focus = NULL;
    layer->interface = &free_layer_interface;
    list_init(&layer->windows);

    return layer;
}

bool layer_is_free(struct velox_layer * layer)
{
    return layer->interface == &free_layer_interface;
}

static void free_layer_update(struct velox_layer * layer)
{
    /* FIXME: Stacking */
}

static void free_layer_destroy(struct velox_layer * layer)
{
    free(layer);
}

static void free_layer_focus_next(struct velox_layer * layer)
{
    if (list_is_trivial(&layer->windows))
        return;

    layer->focus = list_last(&layer->windows, struct velox_window);
    link_move_after(&layer->focus->link, &layer->windows.head);
    /* FIXME: Stacking */
}

static void free_layer_focus_prev(struct velox_layer * layer)
{
    struct velox_window * window;

    if (list_is_trivial(&layer->windows))
        return;

    window = list_first(&layer->windows, struct velox_window);
    link_move_before(&window->link, &layer->windows.head);
    layer->focus = list_first(&layer->windows, struct velox_window);
    /* FIXME: Stacking */
}

/* No-op in the free layer. */
static void free_layer_swap_next(struct velox_layer * layer)
{
}

/* No-op in the free layer. */
static void free_layer_swap_prev(struct velox_layer * layer)
{
}

static void free_layer_add_window(struct velox_layer * layer,
                             struct velox_window * window)
{
    list_prepend(&layer->windows, window);
    layer->focus = window;
}

static void free_layer_remove_window(struct velox_layer * layer,
                                struct velox_window * window)
{
    list_del(window);

    if (window == layer->focus)
    {
        layer->focus = list_is_empty(&layer->windows)
            ? NULL : list_first(&layer->windows, struct velox_window);
    }
}

static void free_layer_set_focus(struct velox_layer * layer,
                            struct velox_window * window)
{
    link_move_after(&window->link, &layer->windows.head);
    layer->focus = window;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

