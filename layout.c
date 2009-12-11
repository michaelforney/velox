/* mwm: layout.c
 *
 * Copyright (c) 2009 Michael Forney <michael@obberon.com>
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

#include "mwm.h"
#include "window.h"
#include "layout.h"

void tile_arrange(struct mwm_window_list * windows, struct mwm_layout_state * generic_state)
{
    struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) generic_state;
    struct mwm_window * window = NULL;
    struct mwm_window_list * current_element = NULL;
    uint16_t mask;
    uint32_t values[4];
    uint16_t window_count = 0;
    uint16_t window_index = 0;
    uint16_t rows_per_column;
    uint16_t column_width;
    uint16_t column_index = 0;
    uint16_t row_index = 0;
    uint16_t row_count;

    printf("tile_arrange\n");

    printf("screen_width: %i, screen_height: %i\n", screen_width, screen_height);

    if (windows == NULL)
    {
        return;
    }

    /* Calculate number of windows */
    for (current_element = windows; current_element != NULL; current_element = current_element->next)
    {
        if (!current_element->window->floating)
        {
            window_count++;
        }
    }

    printf("window_count: %i\n", window_count);

    column_width = ((state->master_count == 0) ? screen_width : (screen_width - (state->master_factor * screen_width))) / MIN(window_count - state->master_count, state->column_count);

    if ((window_count - state->master_count) % state->column_count == 0)
    {
        row_count = (window_count - state->master_count) / state->column_count;
    }
    else
    {
        row_count = ((window_count - state->master_count) / state->column_count) + ((column_index < ((window_count - state->master_count) % state->column_count)) ? 1 : 0);
    }

    /* Arrange the windows */
    for (current_element = windows, window_index = 0; current_element != NULL; current_element = current_element->next)
    {
        window = current_element->window;

        if (window->floating)
        {
            continue;
        }

        if (window_index < state->master_count) /* Arranging a master */
        {
            window->x = 0;
            window->y = window_index * screen_height / MIN(state->master_count, window_count);
            window->width = ((window_count <= state->master_count) ? screen_width : state->master_factor * screen_width) - (2 * window->border_width);
            window->height = screen_height / MIN(state->master_count, window_count) - (2 * window->border_width);
        }
        else /* Arranging the rest of the windows */
        {
            if (row_index == row_count)
            {
                row_index = 0;
                column_index++;
                if ((window_count - state->master_count) % state->column_count == 0)
                {
                    row_count = (window_count - state->master_count) / state->column_count;
                }
                else
                {
                    row_count = ((window_count - state->master_count) / state->column_count) + ((column_index < ((window_count - state->master_count) % state->column_count)) ? 1 : 0);
                }
            }

            window->x = ((state->master_count == 0) ? 0 : state->master_factor * screen_width) + column_index * column_width;
            window->y = screen_height * row_index / row_count;
            window->width = column_width - (2 * window->border_width);
            window->height = screen_height / row_count - (2 * window->border_width);

            row_index++;
        }

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
}

void grid_arrange(struct mwm_window_list * windows, struct mwm_layout_state * generic_state)
{
    struct mwm_window * window = NULL;
    struct mwm_window_list * current_element = NULL;
    uint16_t mask;
    uint32_t values[4];
    uint16_t window_count = 0;
    uint16_t rows = 0;
    uint16_t cols = 0;
    uint16_t row = 0;
    uint16_t col = 0;
    bool perfect = false;

    printf("grid_arrange\n");

    if (windows == NULL)
    {
        return;
    }

    /* Calculate number of windows */
    for (current_element = windows; current_element != NULL; current_element = current_element->next)
    {
        if (!current_element->window->floating)
        {
            window_count++;
        }
    }

    cols = (uint16_t) ceil(sqrt(window_count));
    rows = (window_count > (cols - 1) * cols) ? cols : cols - 1;
    perfect = window_count == (rows * cols);
    printf("rows: %i, cols: %i\n", rows, cols);
    printf("window_count: %i\n", window_count);

    for (current_element = windows; current_element != NULL; current_element = current_element->next)
    {
        window = current_element->window;

        if (window->floating)
        {
            continue;
        }

        if (row >= rows)
        {
            row = 0;
            ++col;
        }

        window->x = col * screen_width / cols;
        window->y = row * screen_height / ((col == cols - 1 && !perfect) ? window_count + rows * (1 - cols) : rows);
        window->width = screen_width / cols - (2 * window->border_width);
        window->height = screen_height / ((col == cols - 1 && !perfect) ? window_count + rows * (1 - cols) : rows) - (2 * window->border_width);

        mask = XCB_CONFIG_WINDOW_X |
               XCB_CONFIG_WINDOW_Y |
               XCB_CONFIG_WINDOW_WIDTH |
               XCB_CONFIG_WINDOW_HEIGHT;

        values[0] = window->x;
        values[1] = window->y;
        values[2] = window->width;
        values[3] = window->height;

        xcb_configure_window(c, window->window_id, mask, values);
        synthetic_configure(window);

        ++row;
    }
}

void setup_layouts()
{
    layouts = (struct mwm_layout *) malloc(LAYOUT_COUNT * sizeof(struct mwm_layout));

    layouts[TILE].identifier = "Tile";
    layouts[TILE].arrange = &tile_arrange;
    memset(&layouts[TILE].default_state, 0, sizeof(struct mwm_layout_state));
    ((struct mwm_tile_layout_state *) &layouts[TILE].default_state)->master_factor = 0.5;
    ((struct mwm_tile_layout_state *) &layouts[TILE].default_state)->master_count = 1;
    ((struct mwm_tile_layout_state *) &layouts[TILE].default_state)->column_count = 1;

    layouts[GRID].identifier = "Grid";
    layouts[GRID].arrange = &grid_arrange;
    memset(&layouts[GRID].default_state, 0, sizeof(struct mwm_layout_state));
}

void cleanup_layouts()
{
    free(layouts);
}

