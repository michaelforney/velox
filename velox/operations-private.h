/* velox: velox/operations-private.h
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

#ifndef VELOX_OPERATIONS_PRIVATE_H
#define VELOX_OPERATIONS_PRIVATE_H

#include <velox/binding.h>

struct velox_binding;
struct velox_window;

/* Window operations */
void show_window(struct velox_window * window);
void hide_window(struct velox_window * window);
void focus_window(struct velox_window * window);

/* Binding operations */
void register_binding(enum velox_binding_type type,
                      struct velox_binding * binding);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

