/* velox: velox/velox.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <mforney@mforney.org>
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

#ifndef VELOX_VELOX_H
#define VELOX_VELOX_H

#include <xcb/xcb.h>
#include <sys/param.h>

#include <velox/window.h>
#include <velox/workspace.h>
#include <velox/binding.h>

extern xcb_connection_t * c;
extern xcb_screen_t * screen;

extern struct velox_area screen_area;
extern struct velox_area work_area;
extern uint16_t border_width;

extern const char wm_name[];

extern xcb_atom_t WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE;

extern struct velox_workspace * workspace;

void synthetic_configure(struct velox_window * window);

void arrange();
void restack();

void spawn(char * const cmd[]);
void spawn_terminal();
void spawn_dmenu();

void focus_next();
void focus_previous();
void focus(xcb_window_t window);
void focus_cursor(union velox_argument);

void move_next();
void move_previous();

void kill_focused_window();
void toggle_floating();

void move_float(union velox_argument argument);
void resize_float(union velox_argument argument);

void next_layout();
void previous_layout();

void die(const char * const message, ...);
void quit();

void set_workspace(union velox_argument argument);
void move_focus_to_workspace(union velox_argument argument);
void next_workspace();
void previous_workspace();

void set_focus_type(enum velox_workspace_focus_type focus_type);
void toggle_focus_type();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

