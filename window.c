/* velox: window.c
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

#include "window.h"
#include "config.h"
#include "screen.h"
#include "tag.h"
#include "velox.h"

#include <stdlib.h>
#include <swc.h>

static uint32_t border_color_active = 0xff338833;
static uint32_t border_color_inactive = 0xff888888;

static bool
border_width_set(struct config_node *node, const char *value)
{
	return config_set_unsigned(&border_width, value, 0);
}

static bool
border_color_active_set(struct config_node *node, const char *value)
{
	return config_set_unsigned(&border_color_active, value, 16);
}

static bool
border_color_inactive_set(struct config_node *node, const char *value)
{
	return config_set_unsigned(&border_color_inactive, value, 16);
}

static void
begin_move(struct config_node *node)
{
	struct window *focus = velox.active_screen->focus;

	if (!focus)
		return;

	window_set_layer(focus, STACK);
	swc_window_begin_move(focus->swc);
}

static void
end_move(struct config_node *node)
{
	struct window *focus = velox.active_screen->focus;

	if (focus)
		swc_window_end_move(focus->swc);
}

static void
begin_resize(struct config_node *node)
{
	struct window *focus = velox.active_screen->focus;

	if (!focus)
		return;

	window_set_layer(focus, STACK);
	swc_window_begin_resize(focus->swc, SWC_WINDOW_EDGE_AUTO);
}

static void
end_resize(struct config_node *node)
{
	struct window *focus = velox.active_screen->focus;

	if (focus)
		swc_window_end_resize(focus->swc);
}

static void
switch_layer(struct config_node *node)
{
	struct window *focus = velox.active_screen->focus;

	if (focus)
		window_set_layer(focus, (focus->layer + 1) % NUM_LAYERS);
}

static void
close_window(struct config_node *node)
{
	struct window *focus = velox.active_screen->focus;

	if (focus)
		swc_window_close(focus->swc);
}

static CONFIG_GROUP(window);
static CONFIG_PROPERTY(border_width, &border_width_set);
static CONFIG_PROPERTY(border_color_active, &border_color_active_set);
static CONFIG_PROPERTY(border_color_inactive, &border_color_inactive_set);
static CONFIG_ACTION(begin_move, &begin_move);
static CONFIG_ACTION(end_move, &end_move);
static CONFIG_ACTION(begin_resize, &begin_resize);
static CONFIG_ACTION(end_resize, &end_resize);
static CONFIG_ACTION(switch_layer, &switch_layer);
static CONFIG_ACTION(close, &close_window);

void
window_add_config_nodes()
{
	wl_list_insert(&window_group.group, &border_width_property.link);
	wl_list_insert(&window_group.group, &border_color_active_property.link);
	wl_list_insert(&window_group.group, &border_color_inactive_property.link);
	wl_list_insert(&window_group.group, &begin_move_action.link);
	wl_list_insert(&window_group.group, &end_move_action.link);
	wl_list_insert(&window_group.group, &begin_resize_action.link);
	wl_list_insert(&window_group.group, &end_resize_action.link);
	wl_list_insert(&window_group.group, &switch_layer_action.link);
	wl_list_insert(&window_group.group, &close_action.link);
	wl_list_insert(config_root, &window_group.link);
}

static void
destroy(void *data)
{
	struct window *window = data;

	unmanage(window);
	free(window);
}

static void
title_changed(void *data)
{
	struct window *window = data;

	/* If this window focused on a screen, make sure bound clients are aware of
	 * this title change. */
	if (window->tag->screen && window->tag->screen->focus == window)
		screen_focus_notify(window->tag->screen);
}

static void
parent_changed(void *data)
{
	struct window *window = data;

	if (window->swc->parent)
		window_set_layer(window, STACK);

	/* TODO: We should probably center this window in the parent. */
}

static void
entered(void *data)
{
	struct window *window = data;

	window_focus(window);
	window->tag->screen->focus = window;
}

static const struct swc_window_handler window_handler = {
	.destroy = &destroy,
	.title_changed = &title_changed,
	.parent_changed = &parent_changed,
	.entered = &entered,
};

struct window *
window_new(struct swc_window *swc)
{
	struct window *window;

	if (!(window = malloc(sizeof *window)))
		return NULL;

	window->swc = swc;
	window->tag = NULL;
	swc_window_set_handler(swc, &window_handler, window);
	window_set_layer(window, TILE);

	return window;
}

void
window_focus(struct window *window)
{
	/* This will become stale if the focused window is destroyed. However, we make
	 * sure to change the focus of a screen in screen_remove_windows when its
	 * focus is removed (before it is actually destroyed). */
	static struct window *focused_window;

	if (focused_window)
		swc_window_set_border(focused_window->swc, border_color_inactive, border_width);

	if (window) {
		swc_window_set_border(window->swc, border_color_active, border_width);
		swc_window_focus(window->swc);
	} else {
		swc_window_focus(NULL);
	}

	focused_window = window;
}

void
window_show(struct window *window)
{
	swc_window_show(window->swc);
}

void
window_hide(struct window *window)
{
	swc_window_hide(window->swc);
}

void
window_set_tag(struct window *window, struct tag *tag)
{
	struct tag *old_tag = window->tag;

	if (tag == old_tag)
		return;

	window->tag = tag;

	if (old_tag)
		tag_update_num_windows(old_tag, -1);
	if (tag)
		tag_update_num_windows(tag, +1);

	/* If the focused window changes tag, but not screen, make sure the
	 * screen notifies any clients of the new tag. */
	if (tag && old_tag && tag->screen && tag->screen == old_tag->screen) {
		if (tag->screen->focus == window)
			screen_focus_notify(tag->screen);
		return;
	}

	if (old_tag && old_tag->screen)
		screen_remove_windows(old_tag->screen);
	if (tag && tag->screen)
		screen_add_windows(tag->screen);
}

void
window_set_layer(struct window *window, int layer)
{
	int old_layer = window->layer;

	if (layer == old_layer)
		return;

	window->layer = layer;

	switch (layer) {
	case TILE:
		swc_window_set_tiled(window->swc);
		break;
	case STACK:
		swc_window_set_stacked(window->swc);
		break;
	}

	if (window->tag && window->tag->screen) {
		--window->tag->screen->num_windows[old_layer];
		++window->tag->screen->num_windows[layer];
		update();
	}
}
