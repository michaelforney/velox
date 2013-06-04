/* velox: velox/x11/atom.h
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

#ifndef VELOX_X11_ATOM_H
#define VELOX_X11_ATOM_H

#include <xcb/xcb.h>

void setup_atoms();
void register_atom(const char * const name, xcb_atom_t * atom);
void sync_atoms();

extern xcb_atom_t WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE;

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

