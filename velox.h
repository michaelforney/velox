/* velox: velox.h
 *
 * Copyright (c) 2014 Michael Forney
 * Copyright (c) 2015 Jente Hidskes
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

#ifndef VELOX_VELOX_H
#define VELOX_VELOX_H

#include <wayland-util.h>

#define NUM_TAGS 9

struct window;

struct rule {
	enum {
		RULE_TYPE_WINDOW_TITLE,
		RULE_TYPE_APP_ID,
	} type;
	char *identifier;
	struct config_node *action;

	struct wl_list link;
};

struct velox {
	struct wl_display *display;
	struct wl_event_loop *event_loop;
	struct screen *active_screen;
	struct wl_list screens;
	struct wl_list hidden_windows;
	struct wl_list unused_tags;
	struct wl_list rules;
	struct tag *tags[NUM_TAGS];

	struct wl_global *global;
};

extern struct velox velox;
extern unsigned border_width;

void manage(struct window *window);
void unmanage(struct window *window);
void arrange();
void update();

struct tag *next_tag(uint32_t *tags);
struct tag *find_unused_tag();

#endif
