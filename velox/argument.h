/* velox: velox/argument.h
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

#ifndef VELOX_ARGUMENT
#define VELOX_ARGUMENT

#include <xcb/xcb.h>

union velox_argument
{
    void * pointer;
    struct velox_window * window;
    xcb_window_t window_id;
    uint32_t uint32;
    uint8_t uint8;
};

static inline union velox_argument uint32_argument(uint32_t value)
{
    return (union velox_argument) { .uint32 = value };
}

static inline union velox_argument uint8_argument(uint8_t value)
{
    return (union velox_argument) { .uint8 = value };
}

static inline union velox_argument pointer_argument(void * value)
{
    return (union velox_argument) { .pointer = value };
}

static inline union velox_argument window_id_argument(xcb_window_t window_id)
{
    return (union velox_argument) { .window_id = window_id };
}

static const union velox_argument no_argument = { 0 };

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

