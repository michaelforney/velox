/* velox: layout.c
 *
 * Copyright (c) 2014 Michael Forney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "layout.h"
#include "config.h"
#include "screen.h"
#include "velox.h"
#include "window.h"

#include <stdlib.h>
#include <swc.h>
#include <sys/param.h>

const unsigned master_max = 16;

struct tile_layout
{
    struct layout base;
    unsigned master_size;
    unsigned num_masters, num_columns;
};

static void increase_master_size(struct config_node * node);
static void decrease_master_size(struct config_node * node);
static void increase_num_masters(struct config_node * node);
static void decrease_num_masters(struct config_node * node);
static void increase_num_columns(struct config_node * node);
static void decrease_num_columns(struct config_node * node);

static struct
{
    struct
    {
        struct config_node group,
                           increase_master_size, decrease_master_size,
                           increase_num_masters, decrease_num_masters,
                           increase_num_columns, decrease_num_columns;
    } config;
} tile = {
    .config = {
        { "tile", CONFIG_NODE_TYPE_GROUP },
        { "increase_master_size", CONFIG_NODE_TYPE_ACTION,
            { .action.run = &increase_master_size } },
        { "decrease_master_size", CONFIG_NODE_TYPE_ACTION,
            { .action.run = &decrease_master_size } },
        { "increase_num_masters", CONFIG_NODE_TYPE_ACTION,
            { .action.run = &increase_num_masters } },
        { "decrease_num_masters", CONFIG_NODE_TYPE_ACTION,
            { .action.run = &decrease_num_masters } },
        { "increase_num_columns", CONFIG_NODE_TYPE_ACTION,
            { .action.run = &increase_num_columns } },
        { "decrease_num_columns", CONFIG_NODE_TYPE_ACTION,
            { .action.run = &decrease_num_columns } },
    }
};

void layout_add_config_nodes()
{
    wl_list_init(&tile.config.group.group);
    wl_list_insert(&tile.config.group.group,
                   &tile.config.increase_master_size.link);
    wl_list_insert(&tile.config.group.group,
                   &tile.config.decrease_master_size.link);
    wl_list_insert(&tile.config.group.group,
                   &tile.config.increase_num_masters.link);
    wl_list_insert(&tile.config.group.group,
                   &tile.config.decrease_num_masters.link);
    wl_list_insert(&tile.config.group.group,
                   &tile.config.increase_num_columns.link);
    wl_list_insert(&tile.config.group.group,
                   &tile.config.decrease_num_columns.link);
    wl_list_insert(config_root, &tile.config.group.link);
}

/* Tile layout */
static void tile_arrange(struct layout * base, struct screen * screen,
                         const struct swc_rectangle * area)
{
    struct tile_layout * layout = (void *) base;
    struct window * window;
    struct swc_rectangle window_area;
    unsigned num_masters, num_grids, num_rows, num_columns,
             row_index, column_index, master_width, grid_width;

    num_masters = MIN(screen->num_windows, layout->num_masters);
    num_grids = screen->num_windows - num_masters;

    if (num_masters == 0)
        return;

    window = wl_container_of(screen->windows.next, window, link);

    if (screen->num_windows > layout->num_masters)
    {
        master_width = area->width * layout->master_size / master_max;
        grid_width = area->width - master_width;
    }
    else
        master_width = area->width;

    window_area.x = area->x + border_width;
    window_area.width = master_width - 2 * border_width;
    window_area.height = area->height / num_masters - 2 * border_width;

    for (row_index = 0; row_index < num_masters; ++row_index)
    {
        window_area.y = area->y + border_width
            + row_index * area->height / num_masters;
        swc_window_set_geometry(window->swc, &window_area);
        window = wl_container_of(window->link.next, window, link);
    }

    if (num_grids == 0)
        return;

    num_columns = MIN(num_grids, layout->num_columns);
    num_rows = num_grids / num_columns + 1;

    for (column_index = 0; column_index < num_columns; ++column_index)
    {
        if (column_index == num_grids % num_columns)
            --num_rows;

        window_area.x = area->x + master_width + border_width
            + column_index * grid_width / num_columns;
        window_area.width = grid_width / num_columns - 2 * border_width;
        window_area.height = area->height / num_rows - 2 * border_width;

        for (row_index = 0; row_index < num_rows; ++row_index)
        {
            window_area.y = area->y + border_width
                + row_index * area->height / num_rows;
            swc_window_set_geometry(window->swc, &window_area);
            window = wl_container_of(window->link.next, window, link);
        }
    }
}

static struct tile_layout * tile_layout(struct layout * base)
{
    if (base->arrange != &tile_arrange)
        return NULL;
    return (void *) base;
}

void increase_master_size(struct config_node * node)
{
    struct tile_layout * layout = tile_layout(velox.active_screen->layout);

    if (layout)
    {
        layout->master_size = MIN(layout->master_size + 1, master_max);
        arrange();
    }
}

void decrease_master_size(struct config_node * node)
{
    struct tile_layout * layout = tile_layout(velox.active_screen->layout);

    if (layout)
    {
        layout->master_size = MAX(layout->master_size - 1, 0);
        arrange();
    }
}

void increase_num_masters(struct config_node * node)
{
    struct tile_layout * layout = tile_layout(velox.active_screen->layout);

    if (layout)
    {
        ++layout->num_masters;
        arrange();
    }
}

void decrease_num_masters(struct config_node * node)
{
    struct tile_layout * layout = tile_layout(velox.active_screen->layout);

    if (layout)
    {
        layout->num_masters = MAX(layout->num_masters - 1, 1);
        arrange();
    }
}

void increase_num_columns(struct config_node * node)
{
    struct tile_layout * layout = tile_layout(velox.active_screen->layout);

    if (layout)
    {
        ++layout->num_columns;
        arrange();
    }
}

void decrease_num_columns(struct config_node * node)
{
    struct tile_layout * layout = tile_layout(velox.active_screen->layout);

    if (layout)
    {
        layout->num_columns = MAX(layout->num_columns - 1, 1);
        arrange();
    }
}

struct layout * tile_layout_new()
{
    struct tile_layout * layout;

    if (!(layout = malloc(sizeof *layout)))
        goto error0;

    layout->base.arrange = &tile_arrange;
    layout->master_size = master_max / 2;
    layout->num_masters = 1;
    layout->num_columns = 1;

    return &layout->base;

  error0:
    return NULL;
}

/* Grid layout */
static void grid_arrange(struct layout * layout, struct screen * screen,
                         const struct swc_rectangle * area)
{
    struct window * window;
    unsigned num_columns, num_rows, column_index, row_index;
    struct swc_rectangle window_area;

    if (screen->num_windows == 0) return;

    num_columns = ceil(sqrt(screen->num_windows));
    num_rows = screen->num_windows / num_columns + 1;
    window = wl_container_of(screen->windows.next, window, link);

    for (column_index = 0; &window->link != &screen->windows; ++column_index)
    {
        if (column_index == screen->num_windows % num_columns)
            --num_rows;

        window_area.x = area->x + border_width
            + area->width * column_index / num_columns;
        window_area.width = area->width / num_columns - 2 * border_width;
        window_area.height = area->height / num_rows - 2 * border_width;

        for (row_index = 0; row_index < num_rows; ++row_index)
        {
            window_area.y = area->y + border_width
                + area->height * row_index / num_rows;

            swc_window_set_geometry(window->swc, &window_area);
            window = wl_container_of(window->link.next, window, link);
        }
    }
}

struct layout * grid_layout_new()
{
    struct layout * layout;

    if (!(layout = malloc(sizeof *layout)))
        goto error0;

    layout->arrange = &grid_arrange;

    return layout;

  error0:
    return NULL;
}

