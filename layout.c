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

#include "mwm.h"
#include "window.h"

void tile_arrange(struct mwm_window_stack * windows)
{
    struct mwm_window * window = NULL;
    struct mwm_window_stack * current_element = NULL;
    uint16_t mask;
    uint32_t values[4];
    uint16_t window_count = 0;
    uint16_t window_index = 0;

    printf("tile_arrange\n");

    printf("screen_width: %i, screen_height: %i\n", screen_width, screen_height);

    if (windows == NULL)
    {
        return;
    }

    /* Arrange the master */
    window = windows->window;

    if (windows->next == NULL)
    {
        window->x = 0;
        window->y = 0;
        window->width = screen_width - (2 * window->border_width);
        window->height = screen_height - (2 * window->border_width);
    }
    else
    {
        window->x = 0;
        window->y = 0;
        window->width = screen_width / 2 - (2 * window->border_width);
        window->height = screen_height - (2 * window->border_width);
    }

    mask = XCB_CONFIG_WINDOW_X |
           XCB_CONFIG_WINDOW_Y |
           XCB_CONFIG_WINDOW_WIDTH |
           XCB_CONFIG_WINDOW_HEIGHT;

    values[0] = window->x;
    values[1] = window->y;
    values[2] = window->width;
    values[3] = window->height;

    printf("arranging master: %i (x: %i, y: %i, width: %i, height: %i)\n", window->window_id, window->x, window->y, window->width, window->height);

    xcb_configure_window(c, window->window_id, mask, values);
    synthetic_configure(window);

    /* Calculate number of windows */
    for (current_element = windows; current_element != NULL; current_element = current_element->next, window_count++);

    /* Arange the rest of the windows */
    for (current_element = windows->next; current_element != NULL; current_element = current_element->next, window_index++)
    {
        window = current_element->window;

        window->x = screen_width / 2;
        window->y = screen_height * window_index / (window_count - 1);
        window->width = screen_width / 2 - (2 * window->border_width);
        window->height = screen_height / (window_count - 1) - (2 * window->border_width);

        mask = XCB_CONFIG_WINDOW_X |
               XCB_CONFIG_WINDOW_Y |
               XCB_CONFIG_WINDOW_WIDTH |
               XCB_CONFIG_WINDOW_HEIGHT;

        values[0] = window->x;
        values[1] = window->y;
        values[2] = window->width;
        values[3] = window->height;

        printf("arranging slave: %i\n", window->window_id);

        xcb_configure_window(c, window->window_id, mask, values);
        synthetic_configure(window);
    }
}

void grid_arrange(struct mwm_window_stack * windows)
{
    struct mwm_window * window = NULL;
    struct mwm_window_stack * current_element = NULL;
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
    for (current_element = windows; current_element != NULL; current_element = current_element->next, window_count++);

    cols = (uint16_t) ceil(sqrt(window_count));
    rows = (window_count > (cols - 1) * cols) ? cols : cols - 1;
    perfect = window_count == (rows * cols);
    printf("rows: %i, cols: %i\n", rows, cols);
    printf("window_count: %i\n", window_count);

    for (current_element = windows; current_element != NULL; current_element = current_element->next)
    {
        if (row >= rows)
        {
            row = 0;
            ++col;
        }

        window = current_element->window;

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

