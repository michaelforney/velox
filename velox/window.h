/* velox: velox/window.h
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

#ifndef VELOX_WINDOW_H
#define VELOX_WINDOW_H

#include <stdbool.h>

#include <xcb/xcb.h>

#include <velox/area.h>
#include <velox/list.h>

struct velox_window
{
    xcb_window_t window_id;

    char name[256];
    char class[256];

    struct velox_workspace * workspace;

    int16_t x, y;
    uint16_t width, height;
    uint16_t border_width;

    bool floating;
};

struct velox_window_entry
{
    struct velox_window * window;
    struct list_head head;
};

bool window_has_protocol(xcb_window_t window, xcb_atom_t protocol);

void window_set_geometry(struct velox_window * window, struct velox_area * area);

void update_name_class(struct velox_window * window);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

