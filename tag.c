/* velox: tag.c
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

#include "tag.h"
#include "layout.h"
#include "screen.h"
#include "util.h"
#include "velox.h"
#include "window.h"
#include "protocol/velox-server-protocol.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <swc.h>

static CONFIG_GROUP(tag);

void
tag_add_config_nodes(void)
{
	wl_list_insert(config_root, &tag_group.link);
}

static bool
set_name(struct config_node *node, const char *value)
{
	struct tag *tag = wl_container_of(node, tag, config.name);
	struct wl_resource *resource;
	char *name;

	if (!(name = strdup(value)))
		return false;

	free(tag->name);
	tag->name = name;

	wl_resource_for_each (resource, &tag->resources)
		velox_tag_send_name(resource, tag->name);

	return true;
}

static void
activate(struct config_node *node, const struct variant *v)
{
	struct tag *tag = wl_container_of(node, tag, config.activate);
	struct screen *screen = velox.active_screen;

	screen->last_mask = screen->mask;
	screen_set_tags(screen, tag->mask);
	update();
}

static void
toggle(struct config_node *node, const struct variant *v)
{
	struct tag *tag = wl_container_of(node, tag, config.toggle);
	struct screen *screen = velox.active_screen;

	screen_set_tags(velox.active_screen, screen->mask ^ tag->mask);
	update();
}

static void
apply(struct config_node *node, const struct variant *v)
{
	struct tag *tag = wl_container_of(node, tag, config.apply);
	struct window *w = window_or_focus(v);

	if (!w)
		return;

	window_set_tag(w, tag);
	update();
}

static void
bind_tag(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
	struct tag *tag = data;
	struct wl_resource *resource;

	if (version >= 1)
		version = 1;

	resource = wl_resource_create(client, &velox_tag_interface, version, id);

	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_destructor(resource, &remove_resource);
	wl_list_insert(&tag->resources, wl_resource_get_link(resource));
	velox_tag_send_name(resource, tag->name);
	velox_tag_send_state(resource, tag->num_windows);
	tag_send_screen(tag, client, resource, NULL);
}

struct tag *
tag_new(unsigned index, const char *name)
{
	struct tag *tag;

	if (!(tag = malloc(sizeof *tag)))
		goto error0;

	if (!(tag->name = strdup(name)))
		goto error1;

	tag->mask = TAG_MASK(index);
	tag->screen = NULL;
	tag->num_windows = 0;
	tag->global = wl_global_create(velox.display, &velox_tag_interface, 1, tag, &bind_tag);

	if (!tag->global)
		goto error2;

	tag->config.group.name = strdup(tag->name);
	tag->config.group.type = CONFIG_NODE_TYPE_GROUP;
	wl_list_init(&tag->config.group.group);
	wl_list_insert(&tag_group.group, &tag->config.group.link);

	tag->config.name.name = "name";
	tag->config.name.type = CONFIG_NODE_TYPE_PROPERTY;
	tag->config.name.property.set = &set_name;
	wl_list_insert(&tag->config.group.group, &tag->config.name.link);

	tag->config.activate.name = "activate";
	tag->config.activate.type = CONFIG_NODE_TYPE_ACTION;
	tag->config.activate.action.run = &activate;
	wl_list_insert(&tag->config.group.group, &tag->config.activate.link);

	tag->config.toggle.name = "toggle";
	tag->config.toggle.type = CONFIG_NODE_TYPE_ACTION;
	tag->config.toggle.action.run = &toggle;
	wl_list_insert(&tag->config.group.group, &tag->config.toggle.link);

	tag->config.apply.name = "apply";
	tag->config.apply.type = CONFIG_NODE_TYPE_ACTION;
	tag->config.apply.action.run = &apply;
	wl_list_insert(&tag->config.group.group, &tag->config.apply.link);

	wl_list_init(&tag->resources);

	return tag;

error2:
	free(tag->name);
error1:
	free(tag);
error0:
	return NULL;
}

void
tag_destroy(struct tag *tag)
{
	free(tag->name);
	free(tag);
}

void
tag_add(struct tag *tag, struct screen *screen)
{
	struct wl_resource *resource;

	assert(tag->screen == NULL);

	/* Add the tag to the end of the tag list to minimize churn of the screen's
	 * active tag, and to prefer recently released tags. */
	wl_list_insert(screen ? screen->tags.prev : &velox.unused_tags, &tag->link);

	if (screen) {
		screen->mask |= tag->mask;
		tag->screen = screen;
	}

	wl_resource_for_each (resource, &tag->resources)
		tag_send_screen(tag, wl_resource_get_client(resource), resource, NULL);
}

void
tag_remove(struct tag *tag, struct screen *screen)
{
	assert(tag->screen == screen);

	wl_list_remove(&tag->link);

	if (screen) {
		screen->mask &= ~tag->mask;
		tag->screen = NULL;
	}
}

void
tag_set(struct tag *tag, struct screen *screen)
{
	if (tag->screen == screen)
		return;

	tag_remove(tag, tag->screen);
	tag_add(tag, screen);
}

void
tag_send_screen(struct tag *tag, struct wl_client *client,
                struct wl_resource *tag_resource, struct wl_resource *screen_resource)
{
	if (!tag_resource)
		tag_resource = wl_resource_find_for_client(&tag->resources, client);

	if (!screen_resource) {
		screen_resource = tag->screen
		                      ? wl_resource_find_for_client(&tag->screen->resources, client)
		                      : NULL;
	}

	velox_tag_send_screen(tag_resource, screen_resource);
}

void
tag_update_num_windows(struct tag *tag, int change)
{
	struct wl_resource *resource;

	tag->num_windows += change;
	wl_resource_for_each (resource, &tag->resources)
		velox_tag_send_state(resource, tag->num_windows);
}
