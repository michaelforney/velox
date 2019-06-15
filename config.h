/* velox: config.h
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

#ifndef VELOX_CONFIG_H
#define VELOX_CONFIG_H

#include <stdbool.h>
#include <wayland-util.h>

enum config_node_type {
	CONFIG_NODE_TYPE_GROUP,
	CONFIG_NODE_TYPE_PROPERTY,
	CONFIG_NODE_TYPE_ACTION
};

struct variant {
	enum {
		VARIANT_WINDOW,
	} type;
	union {
		struct window *window;
	};
};

struct config_node {
	const char *name;
	enum config_node_type type;

	union {
		struct wl_list group;
		struct {
			bool (*set)(struct config_node *node, const char *value);
		} property;
		struct {
			void (*run)(struct config_node *node, const struct variant *v);
		} action;
	};

	struct wl_list link;
};

#define CONFIG_GROUP(n) \
	struct config_node n##_group = { \
		.name = #n, \
		.type = CONFIG_NODE_TYPE_GROUP, \
		.group = { &n##_group.group, &n##_group.group } \
	}
#define CONFIG_PROPERTY(n, func) \
	struct config_node n##_property = { \
		.name = #n, \
		.type = CONFIG_NODE_TYPE_PROPERTY, \
		.property = { .set = func } \
	}
#define CONFIG_ACTION(n, func) \
	struct config_node n##_action = { \
		.name = #n, \
		.type = CONFIG_NODE_TYPE_ACTION, \
		.action = { .run = func } \
	}

bool config_parse(void);
bool config_set_unsigned(unsigned *value, const char *string, int base);

extern struct wl_list *config_root;

#endif
