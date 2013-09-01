/* velox: modules/grid.c
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include <velox/velox.h>
#include <velox/window.h>
#include <velox/layout.h>
#include <velox/debug.h>
#include <velox/area.h>
#include <velox/operations.h>

const char name[] = "grid";

static void grid_arrange(struct velox_area * area, struct velox_list * windows,
    struct velox_layout_state * generic_state);

static struct velox_layout grid_layout = {
    .identifier = "grid",
    .arrange = &grid_arrange,
    .default_state = NULL,
    .default_state_size = 0
};

void setup()
{
    printf("Grid Layout: Initializing...");

    add_layout(&grid_layout);

    printf("done\n");
}

void cleanup()
{
    printf("Grid Layout: Cleaning up...");
    printf("done\n");
}

static void grid_arrange(struct velox_area * area, struct velox_list * windows,
    struct velox_layout_state * generic_state)
{
    /* For looping through the window list */
    struct velox_window * window;

    /* Window counts */
    uint16_t window_count = 0;
    uint16_t column_count;

    /* The current row count */
    uint16_t row_count;

    /* Indices */
    uint16_t index = 0;
    uint16_t column_index = 0;
    uint16_t row_index = 0;

    /* Areas */
    struct velox_area column_area;
    struct velox_area window_area;

    DEBUG_ENTER

    if (list_is_empty(windows)) return;

    /* Calculate number of windows */
    list_for_each_entry(windows, window) ++window_count;

    /* FIXME: Is this the best column count to use? */
    column_count = round(sqrt(window_count));

    /* Arrange the windows */
    window = list_first(windows, struct velox_window);
    for (index = 0, column_index = 0; index < window_count; ++column_index)
    {
        velox_area_split_horizontally(area, column_count, column_index, &column_area);

        if (column_index >= window_count % column_count) row_count = window_count / column_count;
        else row_count = window_count / column_count + 1;

        for (row_index = 0; row_index < row_count;
            ++row_index, window = link_entry_next(window))
        {
            velox_area_split_vertically(&column_area, row_count, row_index, &window_area);
            set_window_geometry(window, &window_area);

            ++index;
        }
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

