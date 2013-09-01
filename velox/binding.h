/* velox: velox/binding.h
 *
 * Copyright (c) 2010 Michael Forney <mforney@mforney.org>
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

#include <velox/function.h>

enum velox_binding_type
{
    VELOX_KEY_BINDING = 0,
    VELOX_BUTTON_BINDING
};

void add_key_binding(const char * group, const char * name,
    velox_function_t function, union velox_argument argument);

void add_button_binding(const char * group, const char * name,
    velox_function_t function);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

