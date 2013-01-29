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

const char name[] = "grid";

static void grid_arrange(struct velox_area * area, struct list_head * windows, struct velox_layout_state * generic_state);

void setup()
{
    struct velox_layout_state state;

    printf("Grid Layout: Initializing...");

    add_layout("grid", &grid_arrange, &state);

    printf("done\n");
}

void cleanup()
{
    printf("Grid Layout: Cleaning up...");
    printf("done\n");
}

static void grid_arrange(struct velox_area * area, struct list_head * windows, struct velox_layout_state * generic_state)
{
    /* For looping through the window list */
    struct velox_window_entry * entry;

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

    if (list_empty(windows)) return;

    /* Calculate number of windows */
    list_for_each_entry(entry, windows, head) ++window_count;

    /* FIXME: Is this the best column count to use? */
    column_count = round(sqrt(window_count));

    /* Arrange the windows */
    entry = list_entry(windows->next, struct velox_window_entry, head);
    for (index = 0, column_index = 0; index < window_count; ++column_index)
    {
        velox_area_split_horizontally(area, column_count, column_index, &column_area);

        if (column_index >= window_count % column_count) row_count = window_count / column_count;
        else row_count = window_count / column_count + 1;

        for (row_index = 0; row_index < row_count;
            ++row_index, entry = list_entry(entry->head.next, struct velox_window_entry, head))
        {
            velox_area_split_vertically(&column_area, row_count, row_index, &window_area);
            window_set_geometry(entry->window, &window_area);
            arrange_window(entry->window);

            ++index;
        }
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

