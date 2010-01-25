/* mwm: mwm/keybinding.h
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

#ifndef MWM_KEYBINDING_H
#define MWM_KEYBINDING_H

#include <xcb/xcb.h>

#include <X11/keysym.h>

#include "list.h"

struct mwm_key
{
    uint16_t modifiers;
    xcb_keysym_t keysym;
};

struct mwm_key_binding
{
    struct mwm_key * key;
    xcb_keycode_t keycode;
    void (* function)();
};

extern struct mwm_list * key_bindings;

void setup_configured_keys();

void setup_key_bindings();
void cleanup_key_bindings();

void add_key_binding(struct mwm_key * key, void (* function)());
void add_configured_key_binding(const char * group, const char * name, void (* function)());

#endif

