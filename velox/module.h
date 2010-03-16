/* velox: velox/module.h
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

#ifndef VELOX_MODULE_H
#define VELOX_MODULE_H

#include <velox/binding.h>

#define MODULE_KEY_BINDING(function, arg) \
    add_configured_key_binding(name, #function, &function, arg)

#define MODULE_WINDOW_BUTTON_BINDING(function, arg) \
    add_configured_window_button_binding(name, #function, &function, arg)

#define MODULE_ROOT_BUTTON_BINDING(function, arg) \
    add_configured_root_button_binding(name, #function, &function, arg)

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

