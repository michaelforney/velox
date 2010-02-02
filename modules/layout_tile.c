/* velox: modules/layout_tile.c
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

#include <velox/velox.h>
#include <velox/module.h>
#include <velox/window.h>
#include <velox/layout.h>

const char const name[] = "layout_tile";

struct velox_tile_layout_state
{
    float master_factor;
    uint16_t master_count;
    uint16_t column_count;
    uint16_t pad[28];
};

void tile_arrange(struct velox_loop * windows, struct velox_layout_state * generic_state);

void initialize()
{
    struct velox_tile_layout_state state;

    printf(">>> layout_tile module\n");

    state.master_factor = 0.5;
    state.master_count = 1;
    state.column_count = 1;

    add_layout("tile", &tile_arrange, (struct velox_layout_state *) &state);

    /* Layout modification */
    MODULE_KEYBINDING(increase_master_factor, NULL)
    MODULE_KEYBINDING(decrease_master_factor, NULL)
    MODULE_KEYBINDING(increase_master_count, NULL)
    MODULE_KEYBINDING(decrease_master_count, NULL)
    MODULE_KEYBINDING(increase_column_count, NULL)
    MODULE_KEYBINDING(decrease_column_count, NULL)
}

void cleanup()
{
    printf("<<< layout_tile module\n");
}

void tile_arrange(struct velox_loop * windows, struct velox_layout_state * generic_state)
{
    struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) generic_state;
    struct velox_window * window = NULL;
    struct velox_loop * iterator = NULL;
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
        if (!((struct velox_window *) iterator->data)->floating)
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
        window = (struct velox_window *) iterator->data;

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

void increase_master_factor()
{
    printf("increase_master_factor()\n");

    if (strcmp(((struct velox_layout *) tag->layout->data)->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_factor = MIN(state->master_factor + 0.025, 1.0);

        arrange();
    }
}

void decrease_master_factor()
{
    printf("decrease_master_factor()\n");

    if (strcmp(((struct velox_layout *) tag->layout->data)->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_factor = MAX(state->master_factor - 0.025, 0.0);

        arrange();
    }
}

void increase_master_count()
{
    printf("increase_master_count()\n");

    if (strcmp(((struct velox_layout *) tag->layout->data)->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_count++;

        arrange();
    }
}

void decrease_master_count()
{
    printf("decrease_master_count()\n");

    if (strcmp(((struct velox_layout *) tag->layout->data)->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_count = MAX(state->master_count - 1, 0);

        arrange();
    }
}

void increase_column_count()
{
    printf("increase_column_count()\n");

    if (strcmp(((struct velox_layout *) tag->layout->data)->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->column_count++;

        arrange();
    }
}

void decrease_column_count()
{
    printf("decrease_column_count()\n");

    if (strcmp(((struct velox_layout *) tag->layout->data)->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->column_count = MAX(state->column_count - 1, 1);

        arrange();
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

