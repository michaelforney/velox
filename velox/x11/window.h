/* velox: velox/x11/window.h
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

#ifndef VELOX_X11_WINDOW_H
#define VELOX_X11_WINDOW_H

#include <stdbool.h>
#include <xcb/xcb.h>

#include <velox/window.h>

bool window_has_protocol(xcb_window_t window, xcb_atom_t protocol);

void update_name_class(struct velox_window * window);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

