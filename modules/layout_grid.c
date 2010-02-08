/* velox: modules/layout_grid.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
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

#include <libvelox/area.h>

#include <velox/velox.h>
#include <velox/window.h>
#include <velox/layout.h>

const char const name[] = "layout_grid";

void grid_arrange(struct velox_area * area, struct velox_loop * windows, struct velox_layout_state * generic_state);

void initialize()
{
    struct velox_layout_state state;

    printf(">>> layout_grid module\n");

    add_layout("grid", &grid_arrange, &state);
}

void cleanup()
{
    printf("<<< layout_grid module\n");
}

void grid_arrange(struct velox_area * area, struct velox_loop * windows, struct velox_layout_state * generic_state)
{
    /* For looping through the window list */
    struct velox_window * window = NULL;
    struct velox_loop * iterator = NULL;

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

    printf("grid_arrange\n");

    if (windows == NULL) return;

    /* Calculate number of windows */
    iterator = windows;
    do
    {
        if (!((struct velox_window *) iterator->data)->floating) window_count++;

        iterator = iterator->next;
    } while (iterator != windows);

    /* FIXME: Is this the best column count to use? */
    column_count = round(sqrt(window_count));

    /* Arrange the windows */
    printf("arranging grid\n");
    for (index = 0, column_index = 0; index < window_count; ++column_index)
    {
        velox_area_split_horizontally(area, column_count, column_index, &column_area);

        if (column_index >= window_count % column_count) row_count = window_count / column_count;
        else row_count = window_count / column_count + 1;

        for (row_index = 0; row_index < row_count; ++row_index, iterator = iterator->next)
        {
            window = (struct velox_window *) iterator->data;

            if (window->floating) continue;

            velox_area_split_vertically(&column_area, row_count, row_index, &window_area);
            window_set_geometry(window, &window_area);
            arrange_window(window);

            ++index;
        }
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

