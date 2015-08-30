/* velox: screen.h
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

#ifndef VELOX_SCREEN_H
#define VELOX_SCREEN_H

#include "tag.h"
#include "layout.h"

#include <wayland-server.h>

struct swc_screen;

struct view {
	uint32_t tags;
};

struct screen {
	struct swc_screen *swc;
	struct wl_listener event_listener;
	struct wl_list link;

	struct wl_list tags;
	uint32_t mask, last_mask;

	struct wl_list layouts;
	struct layout *layout[NUM_LAYERS];

	struct wl_list windows;
	unsigned num_windows[NUM_LAYERS];
	struct window *focus;

	struct wl_list resources;
};

struct screen *screen_new(struct swc_screen *swc);

void screen_arrange(struct screen *screen);
void screen_set_tags(struct screen *screen, uint32_t tags);

void screen_focus_next(struct screen *screen);
void screen_focus_prev(struct screen *screen);
void screen_set_focus(struct screen *screen, struct window *window);

void screen_add_windows(struct screen *screen);
void screen_remove_windows(struct screen *screen);

/* Wayland interface */
struct wl_resource *screen_bind(struct screen *screen,
                                struct wl_client *client, uint32_t id);
void screen_focus_title_notify(struct screen *screen);

#endif
