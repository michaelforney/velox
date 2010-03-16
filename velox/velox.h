/* velox: velox/velox.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
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

#include <velox/window.h>
#include <velox/tag.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

extern xcb_connection_t * c;
extern xcb_window_t root;
extern xcb_screen_t * screen;
extern xcb_get_keyboard_mapping_reply_t * keyboard_mapping;

extern struct velox_area screen_area;
extern struct velox_area work_area;
extern uint16_t border_width;

extern const char wm_name[];

extern xcb_atom_t WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE;

extern struct velox_tag * tag;

void synthetic_configure(struct velox_window * window);

void arrange();
void restack();

void spawn(char * const cmd[]);
void spawn_terminal();
void spawn_dmenu();

void focus_next();
void focus_previous();
void focus(xcb_window_t window);

void move_next();
void move_previous();

void kill_focused_window();

void move_float(void * generic_window_id);
void resize_float(void * generic_window_id);

void next_layout();
void previous_layout();

void die(const char const * message, ...);
void quit();

void set_tag(void * generic_index);
void move_focus_to_tag(void * generic_index);
void next_tag();
void previous_tag();

void set_focus_type(enum velox_tag_focus_type focus_type);
void toggle_focus_type();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

