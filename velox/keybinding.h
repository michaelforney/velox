/* velox: velox/keybinding.h
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

#ifndef VELOX_KEYBINDING_H
#define VELOX_KEYBINDING_H

#include <xcb/xcb.h>

#include <velox/list.h>

struct velox_key
{
    uint16_t modifiers;
    xcb_keysym_t keysym;
};

struct velox_key_binding
{
    struct velox_key key;
    xcb_keycode_t keycode;
    void (* function)(void * arg);
    void * arg;
};

struct velox_key_binding_entry
{
    struct velox_key_binding * key_binding;
    struct list_head head;
};

void add_key_binding(struct velox_key * key, void (* function)(void * arg), void * arg);
void add_configured_key_binding(const char * group, const char * name, void (* function)(void * arg), void * arg);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

