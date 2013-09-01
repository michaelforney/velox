/* velox: velox/window.c
 *
 * Copyright (c) 2009 Michael Forney <mforney@mforney.org>
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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "velox.h"
#include "window.h"
#include "debug.h"

struct velox_window * window_new()
{
    struct velox_window * window;

    window = malloc(sizeof *window);
    window->workspace = NULL;

    return window;
}

void window_set_geometry(struct velox_window * window, struct velox_area * area)
{
    window->x = area->x;
    window->y = area->y;
    window->width = area->width - 2 * window->border_width;
    window->height = area->height - 2 * window->border_width;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

