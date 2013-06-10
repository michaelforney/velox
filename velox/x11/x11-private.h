/* velox: velox/x11-private.h
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

#ifndef VELOX_X11_X11_PRIVATE_H
#define VELOX_X11_X11_PRIVATE_H

extern int x11_fd;

void setup_x11();
void cleanup_x11();

void handle_x11_data();

void manage_existing_windows();
void manage(xcb_window_t window_id);
void unmanage(xcb_window_t window_id);

void grab_buttons();

struct velox_window * lookup_window(xcb_window_t window_id);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

