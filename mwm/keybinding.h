/* mwm: mwm/keybinding.h
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

#ifndef MWM_KEYBINDING_H
#define MWM_KEYBINDING_H

#include <xcb/xcb.h>

#include <X11/keysym.h>

struct mwm_key_binding
{
    uint16_t modifiers;
    xcb_keysym_t keysym;
    xcb_keycode_t keycode;
    void (* function)();
};

struct mwm_key_binding_list
{
    struct mwm_key_binding binding;
    struct mwm_key_binding_list * next;
};

extern struct mwm_key_binding_list * key_bindings;
extern const uint16_t key_binding_count;

void setup_key_bindings();
void cleanup_key_bindings();

void add_key_binding(uint16_t modifiers, xcb_keysym_t keysym, void (* function)());

#endif

