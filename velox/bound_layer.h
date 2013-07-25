/* velox: velox/bound_layer.h
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

#ifndef VELOX_BOUND_LAYER_H
#define VELOX_BOUND_LAYER_H

struct velox_layer;
struct velox_layer_state;

struct velox_layer * bound_layer_new(const char * layout_names[],
                                     uint32_t num_layout_names);

struct velox_layout * bound_layer_get_layout(struct velox_layer * layer);
struct velox_layout_state * bound_layer_get_layout_state
    (struct velox_layer * layer);
void bound_layer_next_layout(struct velox_layer * layer);
void bound_layer_prev_layout(struct velox_layer * layer);

bool layer_is_bound(struct velox_layer * layer);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

