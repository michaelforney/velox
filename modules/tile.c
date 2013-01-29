/* velox: modules/tile.c
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
#include <yaml.h>

#include <velox/velox.h>
#include <velox/module.h>
#include <velox/window.h>
#include <velox/layout.h>
#include <velox/debug.h>
#include <velox/area.h>

const char name[] = "tile";

struct velox_tile_layout_state
{
    float master_factor;
    uint16_t master_count;
    uint16_t column_count;
    uint16_t pad[28];
};

static struct velox_tile_layout_state default_state = {
    0.5,    // Master factor
    1,      // Master count
    1       // Column count
};

static void tile_arrange(struct velox_area * area, struct list_head * windows, struct velox_layout_state * generic_state);

static void increase_master_factor();
static void decrease_master_factor();
static void increase_master_count();
static void decrease_master_count();
static void increase_column_count();
static void decrease_column_count();

void configure(yaml_document_t * document)
{
    yaml_node_t * map;
    yaml_node_pair_t * pair;

    yaml_node_t * key, * value;

    printf("Tile Layout: Loading configuration...");

    map = yaml_document_get_root_node(document);
    assert(map->type == YAML_MAPPING_NODE);

    for (pair = map->data.mapping.pairs.start;
        pair < map->data.mapping.pairs.top;
        ++pair)
    {
        key = yaml_document_get_node(document, pair->key);
        value = yaml_document_get_node(document, pair->value);

        assert(key->type == YAML_SCALAR_NODE);

        if (strcmp((const char *) key->data.scalar.value, "master_factor") == 0)
        {
            assert(value->type == YAML_SCALAR_NODE);
            default_state.master_factor = strtod((const char *) value->data.scalar.value, NULL);
        }
        else if (strcmp((const char *) key->data.scalar.value, "master_count") == 0)
        {
            assert(value->type == YAML_SCALAR_NODE);
            default_state.master_count = strtoul((const char *) value->data.scalar.value, NULL, 10);
        }
        else if (strcmp((const char *) key->data.scalar.value, "column_count") == 0)
        {
            assert(value->type == YAML_SCALAR_NODE);
            default_state.column_count = strtoul((const char *) value->data.scalar.value, NULL, 10);
        }
    }

    printf("done\n    Master factor: %f\n    Master count: %u\n    Column_count: %u\n",
        default_state.master_factor,
        default_state.master_count,
        default_state.column_count
    );
}

bool setup()
{
    printf("Grid Layout: Initializing...");

    add_layout("tile", &tile_arrange, (struct velox_layout_state *) &default_state);

    /* Layout modification */
    MODULE_KEY_BINDING(increase_master_factor, no_argument);
    MODULE_KEY_BINDING(decrease_master_factor, no_argument);
    MODULE_KEY_BINDING(increase_master_count, no_argument);
    MODULE_KEY_BINDING(decrease_master_count, no_argument);
    MODULE_KEY_BINDING(increase_column_count, no_argument);
    MODULE_KEY_BINDING(decrease_column_count, no_argument);

    printf("done\n");

    return true;
}

void cleanup()
{
    printf("Grid Layout: Cleaning up...");
    printf("done\n");
}

static void tile_arrange(struct velox_area * area, struct list_head * windows, struct velox_layout_state * generic_state)
{
    struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) generic_state;

    /* For looping through the window list */
    struct velox_window_entry * entry;

    /* Window counts */
    uint16_t window_count = 0;
    uint16_t master_count; // The *real* master count
    uint16_t column_count; // The *real* column count
    uint16_t grid_count;

    /* The current row count */
    uint16_t row_count;

    /* Indices */
    uint16_t index = 0;
    uint16_t column_index = 0;
    uint16_t row_index = 0;

    /* Areas */
    struct velox_area master_area;
    struct velox_area grid_area;
    struct velox_area grid_column_area;
    struct velox_area window_area;

    DEBUG_ENTER

    if (list_empty(windows)) return;

    /* Calculate number of windows */
    list_for_each_entry(entry, windows, head) ++window_count;

    master_count = MIN(window_count, state->master_count);
    grid_count = window_count - master_count;
    column_count = MIN(grid_count, state->column_count);

    DEBUG_PRINT("window_count: %u\n", window_count);
    DEBUG_PRINT("master_count: %u\n", master_count);
    DEBUG_PRINT("grid_count: %u\n", grid_count);

    /* Set the master and grid areas
     *
     * There is no grid area */
    if (window_count <= state->master_count) master_area = *area;
    /* There is both a master area, and grid area */
    else if (state->master_count > 0)
    {
        master_area = *area;
        master_area.width = state->master_factor * area->width;

        grid_area = *area;
        grid_area.x = master_area.x + master_area.width;
        grid_area.width = area->width - master_area.width;
    }
    /* There is no master area */
    else grid_area = *area;

    /* Arrange the master windows */
    DEBUG_PRINT("arranging masters\n")
    entry = list_entry(windows->next, struct velox_window_entry, head);
    for (index = 0; index < master_count;
        entry = list_entry(entry->head.next, struct velox_window_entry, head))
    {
        velox_area_split_vertically(&master_area, master_count, index, &window_area);
        window_set_geometry(entry->window, &window_area);
        arrange_window(entry->window);

        ++index;
    }

    /* Arrange the grid windows */
    DEBUG_PRINT("arranging grid\n")
    for (index = 0, column_index = 0; index < grid_count; ++column_index)
    {
        velox_area_split_horizontally(&grid_area, column_count, column_index, &grid_column_area);

        if (column_index >= grid_count % column_count) row_count = grid_count / column_count;
        else row_count = grid_count / column_count + 1;

        for (row_index = 0; row_index < row_count;
            ++row_index, entry = list_entry(entry->head.next, struct velox_window_entry, head))
        {
            velox_area_split_vertically(&grid_column_area, row_count, row_index, &window_area);
            window_set_geometry(entry->window, &window_area);
            arrange_window(entry->window);

            ++index;
        }
    }
}

static void increase_master_factor()
{
    DEBUG_ENTER

    if (strcmp(list_entry(
            tag->layout, struct velox_layout_entry, head
        )->layout->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_factor = MIN(state->master_factor + 0.025, 1.0);

        arrange();
    }
}

static void decrease_master_factor()
{
    DEBUG_ENTER

    if (strcmp(list_entry(
            tag->layout, struct velox_layout_entry, head
        )->layout->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_factor = MAX(state->master_factor - 0.025, 0.0);

        arrange();
    }
}

static void increase_master_count()
{
    DEBUG_ENTER

    if (strcmp(list_entry(
            tag->layout, struct velox_layout_entry, head
        )->layout->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_count++;

        arrange();
    }
}

static void decrease_master_count()
{
    DEBUG_ENTER

    if (strcmp(list_entry(
            tag->layout, struct velox_layout_entry, head
        )->layout->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->master_count = MAX(state->master_count - 1, 0);

        arrange();
    }
}

static void increase_column_count()
{
    DEBUG_ENTER

    if (strcmp(list_entry(
            tag->layout, struct velox_layout_entry, head
        )->layout->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->column_count++;

        arrange();
    }
}

static void decrease_column_count()
{
    DEBUG_ENTER

    if (strcmp(list_entry(
            tag->layout, struct velox_layout_entry, head
        )->layout->identifier, "tile") == 0)
    {
        struct velox_tile_layout_state * state = (struct velox_tile_layout_state *) (&tag->state);
        state->column_count = MAX(state->column_count - 1, 1);

        arrange();
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

