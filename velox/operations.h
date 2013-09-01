/* velox: velox/operations.h
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

#ifndef VELOX_OPERATIONS_H
#define VELOX_OPERATIONS_H

struct velox_area;
struct velox_window;

/* Window operations */
void set_window_geometry(struct velox_window * window,
                         struct velox_area * area);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

