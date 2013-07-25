/* velox: velox/layer.c
 *
 * Copyright (c) 2012 Michael Forney <mforney@mforney.org>
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

void layer_update(struct velox_layer * layer)
{
    layer->interface->update(layer);
}

void layer_destroy(struct velox_layer * layer)
{
    layer->interface->destroy(layer);
}

void layer_focus_next(struct velox_layer * layer)
{
    layer->interface->focus_next(layer);
}

void layer_focus_prev(struct velox_layer * layer)
{
    layer->interface->focus_prev(layer);
}

void layer_swap_next(struct velox_layer * layer)
{
    layer->interface->swap_next(layer);
}

void layer_swap_prev(struct velox_layer * layer)
{
    layer->interface->swap_prev(layer);
}

void layer_add_window(struct velox_layer * layer, struct velox_window * window)
{
    layer->interface->add_window(layer, window);
}

void layer_remove_window(struct velox_layer * layer,
                         struct velox_window * window)
{
    layer->interface->remove_window(layer, window);
}

void layer_set_focus(struct velox_layer * layer, struct velox_window * window)
{
    layer->interface->set_focus(layer, window);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

