/* mwm: mwm/layout.c
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
#include <assert.h>

#include "mwm.h"
#include "window.h"
#include "layout.h"

struct mwm_hashtable * layouts;

void tile_arrange(struct mwm_loop * windows, struct mwm_layout_state * generic_state)
{
    struct mwm_tile_layout_state * state = (struct mwm_tile_layout_state *) generic_state;
    struct mwm_window * window = NULL;
    struct mwm_loop * iterator = NULL;
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
    iterator = windows;
    window_index = 0;
    do
    {
        window = (struct mwm_window *) iterator->data;

        if (!window->floating)
        {
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

        iterator = iterator->next;
    } while (iterator != windows);
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

void setup_layouts()
{
    struct mwm_layout * tile, * grid;
    struct mwm_tile_layout_state * tile_state;

    layouts = mwm_hashtable_create(1024, &sdbm_hash);

    tile = (struct mwm_layout *) malloc(sizeof(struct mwm_layout));
    grid = (struct mwm_layout *) malloc(sizeof(struct mwm_layout));

    tile->identifier = "tile";
    tile->arrange = &tile_arrange;
    memset(&tile->default_state, 0, sizeof(struct mwm_layout_state));
    tile_state = (struct mwm_tile_layout_state *) &tile->default_state;
    tile_state->master_factor = 0.5;
    tile_state->master_count = 1;
    tile_state->column_count = 1;

    grid->identifier = "grid";
    grid->arrange = &grid_arrange;
    memset(&grid->default_state, 0, sizeof(struct mwm_layout_state));

    assert(!mwm_hashtable_exists(layouts, tile->identifier));
    mwm_hashtable_insert(layouts, tile->identifier, tile);
    assert(!mwm_hashtable_exists(layouts, grid->identifier));
    mwm_hashtable_insert(layouts, grid->identifier, grid);
}

void cleanup_layouts()
{
    mwm_hashtable_clear(layouts, true);
}

