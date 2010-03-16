/* velox: velox/binding.h
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
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

#ifndef VELOX_BINDING
#define VELOX_BINDING

#include <xcb/xcb.h>

typedef void (* velox_binding_function_t)(void * arg);

enum velox_binding_type
{
    KEY,
    BUTTON
};

struct velox_bindable
{
    union
    {
        struct
        {
            xcb_keycode_t keycode;
            xcb_keysym_t keysym;
        } key;
        uint8_t button;
    } pressable;

    uint16_t modifiers;
};

void add_key_binding(struct velox_bindable * bindable,
    velox_binding_function_t function, void * arg);
void add_window_button_binding(struct velox_bindable * bindable,
    velox_binding_function_t function, void * arg);
void add_root_button_binding(struct velox_bindable * bindable,
    velox_binding_function_t function, void * arg);

void add_configured_key_binding(const char * group, const char * name,
    velox_binding_function_t function, void * arg);

void add_configured_window_button_binding(const char * group, const char * name,
    velox_binding_function_t function);

void add_configured_root_button_binding(const char * group, const char * name,
    velox_binding_function_t function, void * arg);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

