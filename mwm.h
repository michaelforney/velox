/* mwm: mwm.h
 *
 * Copyright (c) 2009 Michael Forney <michael@obberon.com>
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

#ifndef MWM_H
#define MWM_H

#include <xcb/xcb.h>

extern xcb_connection_t * c;

extern uint16_t screen_width;
extern uint16_t screen_height;

void arrange();

void spawn(const char ** cmd);
void spawn_terminal();

void focus_next();
void focus_previous();

void move_next();
void move_previous();

#endif

