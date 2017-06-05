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

struct layout_impl {
	void (*begin)(struct layout *layout, const struct swc_rectangle *area, unsigned num_windows);
	void (*arrange)(struct layout *layout, struct window *window);
};

struct col {
	struct swc_rectangle tile;
	const struct swc_rectangle *area;
	unsigned num_rows, row_index;
	void (*next)(struct col *col);
};

struct grid {
	struct col col;
	unsigned num_windows, num_cols, col_index;
};

struct tall_layout {
	struct layout base;
	unsigned num_masters, num_columns, master_size;
	struct grid grid;
	struct swc_rectangle grid_area;
};

struct grid_layout {
	struct layout base;
	struct grid grid;
};

static void
tile(struct col *col, struct window *window)
{
	col->tile.y = col->area->y + border_width + col->row_index * col->area->height / col->num_rows;
	swc_window_set_geometry(window->swc, &col->tile);

	if (++col->row_index < col->num_rows)
		return;

	col->row_index = 0;
	col->next(col);
}

static void
grid_next(struct col *col)
{
	struct grid *grid = wl_container_of(col, grid, col);

	++grid->col_index;

	if (grid->col_index == grid->num_windows % grid->num_cols) {
		--col->num_rows;
		col->tile.height = col->area->height / col->num_rows - 2 * border_width;
	}

	col->tile.x = col->area->x + border_width + col->area->width * grid->col_index / grid->num_cols;
}

static void
grid(struct grid *grid, const struct swc_rectangle *area, unsigned num_windows, unsigned num_cols)
{
	if (num_windows == 0)
		return;

	grid->num_windows = num_windows;
	grid->num_cols = num_cols;
	grid->col.num_rows = num_windows / num_cols + (num_windows % num_cols > 0);
	grid->col.row_index = 0;
	grid->col_index = 0;
	grid->col.tile.x = area->x + border_width;
	grid->col.tile.width = area->width / num_cols - 2 * border_width;
	grid->col.tile.height = area->height / grid->col.num_rows - 2 * border_width;
	grid->col.area = area;
	grid->col.next = &grid_next;
}

/* Tall layout */
static void
tall_next_col(struct col *col)
{
	struct tall_layout *layout = wl_container_of(col, layout, grid.col);

	grid(&layout->grid, &layout->grid_area, layout->grid.num_windows,
	     MIN(layout->grid.num_windows, layout->num_columns));
}

static void
tall_begin(struct layout *base, const struct swc_rectangle *area, unsigned num_windows)
{
	struct tall_layout *layout = (void *)base;
	unsigned master_width;

	layout->grid.col.num_rows = MIN(num_windows, layout->num_masters);

	if (layout->grid.col.num_rows == 0)
		return;

	layout->grid.num_windows = num_windows - layout->grid.col.num_rows;

	if (num_windows > layout->num_masters) {
		master_width = area->width * layout->master_size / master_max;
		layout->grid_area.x = area->x + master_width;
		layout->grid_area.y = area->y;
		layout->grid_area.height = area->height;
		layout->grid_area.width = area->width - master_width;
	} else
		master_width = area->width;

	layout->grid.col.tile = (struct swc_rectangle){
		.x = area->x + border_width,
		.width = master_width - 2 * border_width,
		.height = area->height / layout->grid.col.num_rows - 2 * border_width,
	};
	layout->grid.col.row_index = 0;
	layout->grid.col.area = area;
	layout->grid.col.next = &tall_next_col;
}

static void
tall_arrange(struct layout *base, struct window *window)
{
	struct tall_layout *layout = (void *)base;

	tile(&layout->grid.col, window);
}

static const struct layout_impl tall_impl = {
	.begin = &tall_begin,
	.arrange = &tall_arrange,
};

static struct tall_layout *
tall_layout(struct layout *base)
{
	if (base->impl != &tall_impl)
		return NULL;
	return (void *)base;
}

struct layout *
tall_layout_new()
{
	struct tall_layout *layout;

	if (!(layout = malloc(sizeof(*layout))))
		goto error0;

	layout->base.impl = &tall_impl;
	layout->master_size = master_max / 2;
	layout->num_masters = 1;
	layout->num_columns = 1;

	return &layout->base;

error0:
	return NULL;
}

/* Grid layout */
static void
grid_begin(struct layout *base, const struct swc_rectangle *area, unsigned num_windows)
{
	struct grid_layout *layout = (void *)base;

	grid(&layout->grid, area, num_windows, ceil(sqrt(num_windows)));
}

static void
grid_arrange(struct layout *base, struct window *window)
{
	struct grid_layout *layout = (void *)base;

	tile(&layout->grid.col, window);
}

static const struct layout_impl grid_impl = {
	.begin = &grid_begin,
	.arrange = &grid_arrange,
};

struct layout *
grid_layout_new()
{
	struct grid_layout *layout;

	if (!(layout = malloc(sizeof(*layout))))
		goto error0;

	layout->base.impl = &grid_impl;

	return &layout->base;

error0:
	return NULL;
}

static void
increase_master_size(struct config_node *node, const struct variant *v)
{
	struct tall_layout *layout;

	if (!(layout = tall_layout(velox.active_screen->layout[TILE])))
		return;

	layout->master_size = MIN(layout->master_size + 1, master_max);
	arrange();
}

static void
decrease_master_size(struct config_node *node, const struct variant *v)
{
	struct tall_layout *layout;

	if (!(layout = tall_layout(velox.active_screen->layout[TILE])))
		return;

	layout->master_size = MAX(layout->master_size - 1, 0);
	arrange();
}

static void
increase_num_masters(struct config_node *node, const struct variant *v)
{
	struct tall_layout *layout;

	if (!(layout = tall_layout(velox.active_screen->layout[TILE])))
		return;

	++layout->num_masters;
	arrange();
}

static void
decrease_num_masters(struct config_node *node, const struct variant *v)
{
	struct tall_layout *layout;

	if (!(layout = tall_layout(velox.active_screen->layout[TILE])))
		return;

	layout->num_masters = MAX(layout->num_masters - 1, 1);
	arrange();
}

static void
increase_num_columns(struct config_node *node, const struct variant *v)
{
	struct tall_layout *layout;

	if (!(layout = tall_layout(velox.active_screen->layout[TILE])))
		return;

	++layout->num_columns;
	arrange();
}

static void
decrease_num_columns(struct config_node *node, const struct variant *v)
{
	struct tall_layout *layout;

	if (!(layout = tall_layout(velox.active_screen->layout[TILE])))
		return;

	layout->num_columns = MAX(layout->num_columns - 1, 1);
	arrange();
}

static struct {
	struct {
		struct config_node group,
		    increase_master_size, decrease_master_size,
		    increase_num_masters, decrease_num_masters,
		    increase_num_columns, decrease_num_columns;
	} config;
} tall = {
	.config = {
	    { "tall", CONFIG_NODE_TYPE_GROUP },
	    { "increase_master_size", CONFIG_NODE_TYPE_ACTION, { .action.run = increase_master_size } },
	    { "decrease_master_size", CONFIG_NODE_TYPE_ACTION, { .action.run = decrease_master_size } },
	    { "increase_num_masters", CONFIG_NODE_TYPE_ACTION, { .action.run = increase_num_masters } },
	    { "decrease_num_masters", CONFIG_NODE_TYPE_ACTION, { .action.run = decrease_num_masters } },
	    { "increase_num_columns", CONFIG_NODE_TYPE_ACTION, { .action.run = increase_num_columns } },
	    { "decrease_num_columns", CONFIG_NODE_TYPE_ACTION, { .action.run = decrease_num_columns } },
	}
};

/* Stack layout */
static void
stack_begin(struct layout *layout, const struct swc_rectangle *area, unsigned num_windows)
{
}

static void
stack_arrange(struct layout *layout, struct window *window)
{
	/* TODO: Place window on top of stack when swc adds support for this. */
}

static const struct layout_impl stack_impl = {
	.begin = &stack_begin,
	.arrange = &stack_arrange,
};

struct layout *
stack_layout_new()
{
	struct layout *layout;

	if (!(layout = malloc(sizeof(*layout))))
		goto error0;

	layout->impl = &stack_impl;

	return layout;

error0:
	return NULL;
}

void
layout_add_config_nodes()
{
	wl_list_init(&tall.config.group.group);
	wl_list_insert(&tall.config.group.group,
	               &tall.config.increase_master_size.link);
	wl_list_insert(&tall.config.group.group,
	               &tall.config.decrease_master_size.link);
	wl_list_insert(&tall.config.group.group,
	               &tall.config.increase_num_masters.link);
	wl_list_insert(&tall.config.group.group,
	               &tall.config.decrease_num_masters.link);
	wl_list_insert(&tall.config.group.group,
	               &tall.config.increase_num_columns.link);
	wl_list_insert(&tall.config.group.group,
	               &tall.config.decrease_num_columns.link);
	wl_list_insert(config_root, &tall.config.group.link);
}

void
layout_begin(struct layout *layout, const struct swc_rectangle *area, unsigned num_windows)
{
	layout->impl->begin(layout, area, num_windows);
}

void
layout_arrange(struct layout *layout, struct window *window)
{
	layout->impl->arrange(layout, window);
}
