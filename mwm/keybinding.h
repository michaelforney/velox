/* mwm: mwm/keybinding.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of mwm.
 *
 * mwm is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * mwm is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with mwm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MWM_KEYBINDING_H
#define MWM_KEYBINDING_H

#include <xcb/xcb.h>

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

void add_key_binding(struct mwm_key * key, void (* function)());
void add_configured_key_binding(const char * group, const char * name, void (* function)());

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

