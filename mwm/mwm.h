/* mwm: mwm/mwm.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MWM_MWM_H
#define MWM_MWM_H

#include <xcb/xcb.h>

#include <mwm/window.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

extern xcb_connection_t * c;

extern uint16_t screen_width;
extern uint16_t screen_height;

extern xcb_atom_t WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE;

void synthetic_configure(struct mwm_window * window);

void arrange();

void spawn(char * const cmd[]);
void spawn_terminal();
void spawn_dmenu();

void focus_next();
void focus_previous();

void move_next();
void move_previous();

void kill_focused_window();

void increase_master_factor();
void decrease_master_factor();
void increase_master_count();
void decrease_master_count();
void increase_column_count();
void decrease_column_count();

void next_layout();
void previous_layout();

void quit();

void set_tag(uint8_t index);
void move_focus_to_tag(uint8_t index);

#endif

