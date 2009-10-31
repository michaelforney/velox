/* mwm: window.c
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

#include "window.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint16_t window_table_size = 1024;

struct mwm_window ** windows = NULL;

static void window_table_realloc()
{
    uint16_t new_size;

    /* Reallocation method */
    new_size = window_table_size * 2;

    windows = (struct mwm_window **) realloc(windows, new_size * sizeof(struct mwm_window *));
    memset(windows + window_table_size, NULL, new_size - window_table_size);
    window_table_size = new_size;
}

void window_initialize()
{
    windows = (struct mwm_window **) malloc(window_table_size * sizeof(struct mwm_window *));
}

struct mwm_window * window_lookup(xcb_window_t window_id)
{
    int index;

    for (index = 0; index < window_table_size; index++)
    {
        if (windows[index] != NULL && windows[index]->window_id == window_id)
        {
            return windows[index];
        }
    }

    return NULL;
}

void window_insert(struct mwm_window * window)
{
    int index;

    for (index = 0; index < window_table_size; index++)
    {
        if (windows[index] == NULL)
        {
            windows[index] = window;
            return;
        }
    }

    window_table_size *= 2;
    window_insert(window);
}

void window_delete(xcb_window_t window_id)
{
    int index;

    for (index = 0; index < window_table_size; index++)
    {
        if (windows[index] != NULL && windows[index]->window_id == window_id)
        {
            windows[window_id] = NULL;
            return;
        }
    }
}

