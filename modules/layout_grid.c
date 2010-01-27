// vim: fdm=syntax fo=croql noet sw=4 sts=4 ts=8

/* mwm: modules/layout_grid.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include <mwm/mwm.h>
#include <mwm/window.h>
#include <mwm/layout.h>

const char const name[] = "layout_grid";

void grid_arrange(struct mwm_loop * windows, struct mwm_layout_state * generic_state);

void initialize()
{
    struct mwm_layout_state state;

    printf(">>> layout_grid module\n");

    add_layout("grid", &grid_arrange, &state);
}

void cleanup()
{
    printf("<<< layout_grid module\n");
}

void grid_arrange(struct mwm_loop * windows, struct mwm_layout_state * generic_state)
{
    struct mwm_window * window = NULL;
    struct mwm_loop * iterator = NULL;
    uint16_t mask;
    uint32_t values[4];
    uint16_t window_count = 0;
    uint16_t window_index = 0;
    uint16_t rows_per_column;
    uint16_t column_count;
    uint16_t column_width;
    uint16_t column_index = 0;
    uint16_t row_index = 0;
    uint16_t row_count;

    printf("grid_arrange\n");

    printf("screen_width: %i, screen_height: %i\n", screen_width, screen_height);

    if (windows == NULL)
    {
        return;
    }

    /* Calculate number of windows */
    iterator = windows;
    do
    {
        if (!((struct mwm_window *) iterator->data)->floating)
        {
            window_count++;
        }

        iterator = iterator->next;
    } while (iterator != windows);

    printf("window_count: %i\n", window_count);

    column_count = (uint16_t) floor(sqrt(window_count + 2));
    column_width = screen_width / MIN(window_count, column_count);

    if ((window_count) % column_count == 0)
    {
        row_count = window_count / column_count;
    }
    else
    {
        row_count = (window_count / column_count) + ((column_index < (window_count % column_count)) ? 1 : 0);
    }

    /* Arrange the windows */
    iterator = windows;
    window_index = 0;
    do
    {
        window = (struct mwm_window *) iterator->data;

        if (!window->floating)
        {
            if (row_index == row_count)
            {
                row_index = 0;
                column_index++;
                if (window_count % column_count == 0)
                {
                    row_count = window_count / column_count;
                }
                else
                {
                    row_count = (window_count / column_count) + ((column_index < (window_count % column_count)) ? 1 : 0);
                }
            }

            window->x = column_index * column_width;
            window->y = screen_height * row_index / row_count;
            window->width = column_width - (2 * window->border_width);
            window->height = screen_height / row_count - (2 * window->border_width);

            row_index++;

            mask = XCB_CONFIG_WINDOW_X |
                   XCB_CONFIG_WINDOW_Y |
                   XCB_CONFIG_WINDOW_WIDTH |
                   XCB_CONFIG_WINDOW_HEIGHT;

            values[0] = window->x;
            values[1] = window->y;
            values[2] = window->width;
            values[3] = window->height;

            printf("arranging window: %i (x: %i, y: %i, width: %i, height: %i)\n", window->window_id, window->x, window->y, window->width, window->height);

            xcb_configure_window(c, window->window_id, mask, values);
            synthetic_configure(window);

            window_index++;
        }

        iterator = iterator->next;
    } while (iterator != windows);
}

