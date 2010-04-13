/* velox: velox/binding-private.h
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

#ifndef VELOX_BINDING_PRIVATE
#define VELOX_BINDING_PRIVATE

#include "binding.h"
#include "vector.h"

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

struct velox_binding
{
    struct velox_bindable bindable;
    velox_binding_function_t function;
    void * arg;
};

DEFINE_VECTOR(velox_binding_vector, struct velox_binding);

extern struct velox_binding_vector key_bindings;
extern struct velox_binding_vector window_button_bindings;
extern struct velox_binding_vector root_button_bindings;

void setup_bindings();
void cleanup_bindings();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

