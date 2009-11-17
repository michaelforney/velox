/* mwm: keys.c
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

#ifndef KEYS_H
#define KEYS_H

#include <xcb/xcb.h>

#include <X11/keysym.h>

static const xcb_mod_mask_t mod_mask = XCB_MOD_MASK_4;

struct mwm_key_binding
{
    uint16_t modifiers;
    xcb_keysym_t keysym;
    xcb_keycode_t keycode;
    void (* function)();
};

extern struct mwm_key_binding key_bindings[];

#endif
