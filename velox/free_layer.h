/* velox: velox/free_layer.h
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

#ifndef VELOX_FREE_LAYER_H
#define VELOX_FREE_LAYER_H

struct velox_layer;

struct velox_layer * free_layer_new();

bool layer_is_free(struct velox_layer * layer);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

