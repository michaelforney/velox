/* velox: velox/x11.h
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

#ifndef VELOX_X11_H
#define VELOX_X11_H

#include <xcb/xcb.h>

#include <velox/x11/window.h>

extern xcb_connection_t * c;
extern xcb_screen_t * screen;

extern uint32_t border_pixel;
extern uint32_t border_focus_pixel;

extern uint8_t clear_event_type;

void kill_focused_window();

void show_x11_window(struct velox_window * window);
void hide_x11_window(struct velox_window * window);

void focus_x11_window(struct velox_window * window);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

