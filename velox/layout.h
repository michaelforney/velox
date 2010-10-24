/* velox: velox/layout.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <mforney@mforney.org>
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

#ifndef VELOX_LAYOUT_H
#define VELOX_LAYOUT_H

#include <stdint.h>

#include <velox/area.h>
#include <velox/hashtable.h>
#include <velox/window.h>

struct velox_layout_state
{
    uint16_t pad[32];
};

typedef void (* velox_arrange_t)(
    struct velox_area * area,
    struct list_head * windows,
    struct velox_layout_state * state
);

struct velox_layout
{
    char * identifier;
    velox_arrange_t arrange;
    struct velox_layout_state default_state;
};

struct velox_layout_entry
{
    struct velox_layout * layout;
    struct list_head head;
};

void add_layout(const char const * identifier, velox_arrange_t arrange, struct velox_layout_state * default_state);

void arrange_window(struct velox_window * window);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

