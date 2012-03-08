/* velox: velox/keyboard_mapping.c
 *
 * Copyright (c) 2012 Michael Forney <mforney@mforney.org>
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

#include "velox.h"
#include "keyboard_mapping.h"

xcb_key_symbols_t * keyboard_mapping;

void setup_keyboard_mapping()
{
    keyboard_mapping = xcb_key_symbols_alloc(c);
}

void cleanup_keyboard_mapping()
{
    xcb_key_symbols_free(keyboard_mapping);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

