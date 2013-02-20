/* velox: velox/layout.c
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "velox.h"
#include "window.h"
#include "layout.h"
#include "resource.h"
#include "debug.h"

uint32_t layout_resource_id;

static void free_layout(void * data)
{
    struct velox_layout * layout = data;

    free(layout->identifier);
    free(layout);
}

void setup_layouts()
{
    layout_resource_id = resource_id("layout");
    resource_set_destroy(layout_resource_id, &free_layout);
}

void add_layout(const char * const identifier, velox_arrange_t arrange,
    struct velox_layout_state * default_state)
{
    struct velox_layout * layout;

    /* Create a new layout and set its fields */
    layout = (struct velox_layout *) malloc(sizeof(struct velox_layout));

    layout->identifier = strdup(identifier);
    layout->arrange = arrange;
    layout->default_state = *default_state;

    add_resource(layout_resource_id, layout);
}

struct velox_layout * find_layout(const char * identifier)
{
    struct velox_layout ** layout;
    const struct velox_vector * layouts = get_resources(layout_resource_id);

    vector_for_each(layouts, layout)
    {
        if (strcmp((*layout)->identifier, identifier) == 0)
            return *layout;
    }

    return NULL;
}

void arrange_window(struct velox_window * window)
{
    static uint16_t mask = XCB_CONFIG_WINDOW_X |
                           XCB_CONFIG_WINDOW_Y |
                           XCB_CONFIG_WINDOW_WIDTH |
                           XCB_CONFIG_WINDOW_HEIGHT;
    static uint32_t values[4];

    DEBUG_ENTER

    values[0] = window->x;
    values[1] = window->y;
    values[2] = window->width;
    values[3] = window->height;

    DEBUG_PRINT("0x%x (x: %i, y: %i, width: %i, height: %i)\n",
        window->window_id,
        window->x,
        window->y,
        window->width,
        window->height
    );

    xcb_configure_window(c, window->window_id, mask, values);
    synthetic_configure(window);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

