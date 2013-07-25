/* velox: velox/layer.h
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

#ifndef VELOX_LAYER_H
#define VELOX_LAYER_H

#include <velox/list.h>

struct velox_window;
struct velox_layer;

struct velox_layer_interface
{
    const char * identifier;

    void (* update)(struct velox_layer * layer);
    void (* destroy)(struct velox_layer * layer);
    void (* focus_next)(struct velox_layer * layer);
    void (* focus_prev)(struct velox_layer * layer);
    void (* swap_next)(struct velox_layer * layer);
    void (* swap_prev)(struct velox_layer * layer);
    void (* add_window)(struct velox_layer * layer,
                        struct velox_window * window);
    void (* remove_window)(struct velox_layer * layer,
                           struct velox_window * window);
    void (* set_focus)(struct velox_layer * layer,
                       struct velox_window * window);
};

struct velox_layer
{
    const struct velox_layer_interface * interface;
    struct velox_window * focus;
    struct velox_list windows;

    struct velox_link link;
};

typedef bool (* layer_predicate_t)(struct velox_layer * layer);

void layer_update(struct velox_layer * layer);
void layer_destroy(struct velox_layer * layer);
void layer_focus_next(struct velox_layer * layer);
void layer_focus_prev(struct velox_layer * layer);
void layer_swap_next(struct velox_layer * layer);
void layer_swap_prev(struct velox_layer * layer);
void layer_add_window(struct velox_layer * layer, struct velox_window * window);
void layer_remove_window(struct velox_layer * layer,
                         struct velox_window * window);
void layer_set_focus(struct velox_layer * layer, struct velox_window * window);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

