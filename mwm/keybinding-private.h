/* mwm: mwm/keybinding-private.h
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
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

#ifndef MWM_KEYBINDING_PRIVATE_H
#define MWM_KEYBINDING_PRIVATE_H

#include <xcb/xcb.h>

#include <libmwm/list.h>

extern struct mwm_list * key_bindings;

void setup_configured_keys();

void setup_key_bindings();
void cleanup_key_bindings();

#endif

