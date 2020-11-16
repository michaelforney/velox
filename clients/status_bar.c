/* velox: clients/status_bar.c
 *
 * Copyright (c) 2010, 2013, 2014 Michael Forney <mforney@mforney.org>
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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wld/wayland.h>
#include <wld/wld.h>

#include "protocol/swc-client-protocol.h"
#include "protocol/velox-client-protocol.h"

enum align {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
};

struct item {
	const struct item_interface *interface;
	const struct item_data *data;
	struct wl_list link;
};

struct item_data {
	uint32_t width;
};

struct text_item_data {
	struct item_data base;
	const char *text;
};

struct status_bar {
	struct wl_surface *surface;
	struct swc_panel *panel;

	struct wld_surface *wld_surface;
	uint32_t width, height;

	struct wl_list items[3];
};

struct item_interface {
	void (*draw)(struct status_bar *status_bar, struct item *item, uint32_t x, uint32_t y);
};

struct style {
	uint32_t fg, bg;
};

struct tag {
	struct velox_tag *velox;
	struct velox_screen *screen;
	struct wl_list link;

	struct text_item_data name_data;
	char *name;
	unsigned num_windows;
};

struct screen {
	struct swc_screen *swc;
	struct velox_screen *velox;
	struct status_bar status_bar;
	struct wl_list link;

	struct text_item_data focus_data;
	struct {
		char *title;
		struct velox_tag *tag;
	} focus;
};

/* Wayland listeners */
static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *implementation, uint32_t version);
static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t name);

static void panel_docked(void *data, struct swc_panel *panel, uint32_t length);

static void velox_screen_focus(void *data, struct velox_screen *velox_screen, const char *title, struct velox_tag *tag);
static void velox_tag_name(void *data, struct velox_tag *tag, const char *name);
static void velox_tag_state(void *data, struct velox_tag *tag, uint32_t num_windows);
static void velox_tag_screen(void *data, struct velox_tag *tag, struct velox_screen *screen);

/* Item interfaces */
static void text_draw(struct status_bar *status_bar, struct item *item, uint32_t x, uint32_t y);
static void tag_draw(struct status_bar *status_bar, struct item *item, uint32_t x, uint32_t y);
static void divider_draw(struct status_bar *status_bar, struct item *item, uint32_t x, uint32_t y);

static struct wl_display *display;
static struct wl_registry *registry;
static struct wl_compositor *compositor;
static struct swc_panel_manager *panel_manager;
static struct velox *velox;

static struct wl_list screens, tags;

static struct {
	struct wld_context *context;
	struct wld_renderer *renderer;
	struct wld_font_context *font_context;
	struct wld_font *font;
} wld;

static const struct wl_registry_listener registry_listener = {
	.global = &registry_global,
	.global_remove = &registry_global_remove
};

static const struct swc_panel_listener panel_listener = {
	.docked = &panel_docked
};

static const struct velox_screen_listener velox_screen_listener = {
	.focus = &velox_screen_focus,
};

static const struct velox_tag_listener velox_tag_listener = {
	.name = &velox_tag_name,
	.state = &velox_tag_state,
	.screen = &velox_tag_screen,
};

static const struct item_interface text_interface = {
	.draw = &text_draw
};

static const struct item_interface tag_interface = {
	.draw = &tag_draw
};

static const struct item_interface divider_interface = {
	.draw = &divider_draw
};

/* Configuration parameters */
static const int spacing = 12;
static const char *const font_name = "Terminus:pixelsize=14";
static const struct style normal = { .bg = 0xff1a1a1a, .fg = 0xff999999 };
static const struct style selected = { .bg = 0xff338833, .fg = 0xffffffff };

static timer_t timer;
static bool running, need_draw;
static char clock_text[32];
static struct item_data divider_data = {.width = 14 };
static struct text_item_data clock_data = {.text = clock_text };
static int pfd[2];

static void __attribute__((noreturn)) die(const char *const format, ...)
{
	va_list args;

	va_start(args, format);
	fputs("FATAL: ", stderr);
	vfprintf(stderr, format, args);
	fputc('\n', stderr);
	va_end(args);
	exit(EXIT_FAILURE);
}

static void *
xmalloc(size_t size)
{
	void *data;

	if (!(data = malloc(size)))
		die("Allocation failed");
	return data;
}

static struct item *
item_new(const struct item_interface *interface, const struct item_data *data)
{
	struct item *item;

	if (!(item = malloc(sizeof(*item))))
		die("Failed to allocate item");

	item->interface = interface;
	item->data = data;

	return item;
}

static void
update_text_item_data(struct text_item_data *data)
{
	struct wld_extents extents;

	wld_font_text_extents(wld.font, data->text, &extents);
	data->base.width = extents.advance + spacing;
	need_draw = true;
}

/* Wayland event handlers */
static void
registry_global(void *data, struct wl_registry *registry,
                uint32_t name, const char *interface, uint32_t version)
{
	if (strcmp(interface, "wl_compositor") == 0) {
		compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 3);
	} else if (strcmp(interface, "swc_panel_manager") == 0) {
		panel_manager = wl_registry_bind(registry, name, &swc_panel_manager_interface, 1);
	} else if (strcmp(interface, "swc_screen") == 0) {
		struct screen *screen;

		screen = xmalloc(sizeof(*screen));
		screen->focus_data.text = "";
		screen->focus.title = NULL;
		screen->focus.tag = NULL;
		screen->swc = wl_registry_bind(registry, name, &swc_screen_interface, 1);
		if (!screen->swc)
			die("Failed to bind swc_screen");
		wl_list_insert(screens.prev, &screen->link);
	} else if (strcmp(interface, "velox") == 0) {
		velox = wl_registry_bind(registry, name, &velox_interface, 1);
	} else if (strcmp(interface, "velox_tag") == 0) {
		struct tag *tag;

		tag = xmalloc(sizeof(*tag));
		tag->name = NULL;
		tag->name_data.text = "";
		tag->name_data.base.width = 0;
		tag->screen = NULL;
		tag->num_windows = 0;
		tag->velox = wl_registry_bind(registry, name, &velox_tag_interface, 1);
		if (!tag->velox)
			die("Failed to bind velox_tag");
		wl_list_insert(tags.prev, &tag->link);
		velox_tag_add_listener(tag->velox, &velox_tag_listener, tag);
	}
}

static void
registry_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
}

static void
panel_docked(void *data, struct swc_panel *panel, uint32_t length)
{
	struct status_bar *bar = data;

	bar->width = length;
	bar->height = wld.font->height + 2;
	bar->wld_surface = wld_wayland_create_surface(wld.context, bar->width, bar->height,
	                                              WLD_FORMAT_XRGB8888, 0, bar->surface);
}

static void
velox_tag_name(void *data, struct velox_tag *velox_tag, const char *name)
{
	struct tag *tag = data;

	free(tag->name);
	tag->name = strdup(name);
	tag->name_data.text = tag->name;
	update_text_item_data(&tag->name_data);
}

static void
velox_tag_state(void *data, struct velox_tag *velox_tag, uint32_t num_windows)
{
	struct tag *tag = data;

	tag->num_windows = num_windows;
}

static void
velox_tag_screen(void *data, struct velox_tag *velox_tag, struct velox_screen *velox_screen)
{
	struct tag *tag = data;

	tag->screen = velox_screen;
	need_draw = true;
}

static void
velox_screen_focus(void *data, struct velox_screen *velox_screen, const char *title, struct velox_tag *tag)
{
	struct screen *screen = data;

	free(screen->focus.title);

	if (title) {
		screen->focus.title = strdup(title);
		title = screen->focus.title ? screen->focus.title : "";
	} else {
		screen->focus.title = NULL;
		title = "";
	}

	screen->focus.tag = tag;
	screen->focus_data.text = title;
	update_text_item_data(&screen->focus_data);
}

/* Item implementations */
void
text_draw(struct status_bar *bar, struct item *item, uint32_t x, uint32_t y)
{
	struct text_item_data *data = (void *)item->data;

	wld_draw_text(wld.renderer, wld.font, normal.fg, x, y + wld.font->ascent + 1, data->text, -1, NULL);
}

void
divider_draw(struct status_bar *bar, struct item *item, uint32_t x, uint32_t y)
{
	wld_fill_rectangle(wld.renderer, normal.fg, x + spacing / 2, y, 2, bar->height);
}

void
tag_draw(struct status_bar *bar, struct item *item, uint32_t x, uint32_t y)
{
	struct tag *tag = wl_container_of(item->data, tag, name_data);
	struct screen *screen = wl_container_of(bar, screen, status_bar);
	const struct style *style;

	style = tag->screen == screen->velox ? &selected : &normal;
	wld_fill_rectangle(wld.renderer, style->bg, x, y, item->data->width, bar->height);
	if (tag->num_windows > 0)
		wld_fill_rectangle(wld.renderer, style->fg, x, y, item->data->width, 1);
	if (tag->velox == screen->focus.tag) {
		wld_fill_rectangle(wld.renderer, style->fg, x, y + 1, 3, 1);
		wld_fill_rectangle(wld.renderer, style->fg, x, y + 2, 2, 1);
		wld_fill_rectangle(wld.renderer, style->fg, x, y + 3, 1, 1);
	}

	x += spacing / 2;
	y += wld.font->ascent + 1;
	wld_draw_text(wld.renderer, wld.font, style->fg, x, y, tag->name_data.text, -1, NULL);
}

static void
draw(struct status_bar *bar)
{
	struct item *item;
	uint32_t start_x[3] = {
		0, bar->width / 2, bar->width
	};
	uint32_t x;

	wld_set_target_surface(wld.renderer, bar->wld_surface);
	wld_fill_rectangle(wld.renderer, normal.bg, 0, 0, bar->width, bar->height);

	wl_list_for_each (item, &bar->items[ALIGN_CENTER], link)
		start_x[ALIGN_CENTER] -= item->data->width / 2;

	wl_list_for_each (item, &bar->items[ALIGN_RIGHT], link)
		start_x[ALIGN_RIGHT] -= item->data->width;

	x = start_x[ALIGN_LEFT];
	wl_list_for_each (item, &bar->items[ALIGN_LEFT], link) {
		item->interface->draw(bar, item, x, 0);
		x += item->data->width;
	}

	x = start_x[ALIGN_CENTER];
	wl_list_for_each (item, &bar->items[ALIGN_CENTER], link) {
		item->interface->draw(bar, item, x, 0);
		x += item->data->width;
	}

	x = start_x[ALIGN_RIGHT];
	wl_list_for_each (item, &bar->items[ALIGN_RIGHT], link) {
		item->interface->draw(bar, item, x, 0);
		x += item->data->width;
	}

	wl_surface_damage(bar->surface, 0, 0, bar->width, bar->height);
	wld_flush(wld.renderer);
	wld_swap(bar->wld_surface);
}

static void
setup(void)
{
	struct status_bar *status_bar;
	struct screen *screen;
	struct tag *tag;
	struct wl_list *items;
	struct item *item;

	fprintf(stderr, "status bar: Initializing...");

	wl_list_init(&screens);
	wl_list_init(&tags);

	if (timer_create(CLOCK_MONOTONIC, NULL, &timer) != 0)
		die("Failed to create timer: %s", strerror(errno));

	if (!(display = wl_display_connect(NULL)))
		die("Failed to connect to display");

	if (!(registry = wl_display_get_registry(display)))
		die("Failed to get registry");

	wl_registry_add_listener(registry, &registry_listener, NULL);

	/* Wait for globals. */
	wl_display_roundtrip(display);

	if (!compositor || !panel_manager || !velox) {
		die("Missing required globals: wl_compositor, swc_panel_manager, velox");
	}

	wld.context = wld_wayland_create_context(display, WLD_ANY);
	if (!wld.context)
		die("Failed to create WLD context");

	wld.renderer = wld_create_renderer(wld.context);
	if (!wld.renderer)
		die("Failed to create WLD renderer");

	/* Font */
	wld.font_context = wld_font_create_context();
	if (!wld.font_context)
		die("Failed to create WLD font context");
	wld.font = wld_font_open_name(wld.font_context, font_name);
	if (!wld.font)
		die("Failed to open font");

	/* Create the panels */
	wl_list_for_each (screen, &screens, link) {
		screen->velox = velox_get_screen(velox, screen->swc);
		if (!screen->velox)
			die("Failed to get velox_screen");
		velox_screen_add_listener(screen->velox, &velox_screen_listener, screen);

		status_bar = &screen->status_bar;
		status_bar->surface = wl_compositor_create_surface(compositor);
		status_bar->panel = swc_panel_manager_create_panel(panel_manager, status_bar->surface);
		swc_panel_add_listener(status_bar->panel, &panel_listener, status_bar);
		swc_panel_dock(status_bar->panel, SWC_PANEL_EDGE_TOP, screen->swc, false);

		/* Add items */
		items = &screen->status_bar.items[ALIGN_LEFT];
		wl_list_init(items);

		/* Tags */
		wl_list_for_each (tag, &tags, link) {
			item = item_new(&tag_interface, &tag->name_data.base);
			wl_list_insert(items->prev, &item->link);
		}

		/* Divider */
		item = item_new(&divider_interface, &divider_data);
		wl_list_insert(items->prev, &item->link);

		/* Window title */
		item = item_new(&text_interface, &screen->focus_data.base);
		wl_list_insert(items->prev, &item->link);

		items = &screen->status_bar.items[ALIGN_CENTER];
		wl_list_init(items);

		items = &screen->status_bar.items[ALIGN_RIGHT];
		wl_list_init(items);

		/* Clock */
		item = item_new(&text_interface, &clock_data.base);
		wl_list_insert(items, &item->link);
	}

	/* Wait for dock notifications. */
	wl_display_roundtrip(display);

	wl_list_for_each (screen, &screens, link) {
		if (!screen->status_bar.wld_surface)
			die("");
		swc_panel_set_strut(screen->status_bar.panel, screen->status_bar.height, 0, screen->status_bar.width);
	}

	fprintf(stderr, "done\n");

	wl_display_flush(display);
}

static void
sigalrm_handler(int sig)
{
	int p_errno = errno; /* Preserve errno */
	int ret;

	if ((ret = write(pfd[1], ".", 1)) != 1)
		die("self-pipe write failed\n");
	errno = p_errno;
}

static int
set_nonblock(int fd)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFL)) == -1) {
		return -1;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		return -1;
	}
	return 0;
}


static void
run(void)
{
	struct itimerspec timer_value = {
		.it_interval = { 1, 0 },
		.it_value = { 0, 1 }
	};
	struct pollfd fds[2];
	struct screen *screen;
	struct sigaction sa = {0};

	if (pipe(pfd) == -1)
		die("creating pipe failed: %s\n", strerror(errno));

	if (set_nonblock(pfd[0]) == -1)
		die("could not set O_NONBLOCK on pipe fd: %s\n", strerror(errno));

	if (set_nonblock(pfd[1]) == -1)
		die("could not set O_NONBLOCK on pipe fd\n", strerror(errno));

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sigalrm_handler;

	if (sigaction(SIGALRM, &sa, NULL) == -1)
		die("sigaction failed\n", strerror(errno));

	fds[0].fd = wl_display_get_fd(display);
	fds[0].events = POLLIN;
	fds[1].fd = pfd[0];
	fds[1].events = POLLIN;

	timer_settime(timer, 0, &timer_value, NULL);
	running = true;

	while (true) {
		if (poll(fds, 2, -1) == -1) {
			if (errno != EINTR) {
				perror("poll");
				break;
			}
		}

		if (fds[0].revents & POLLIN) {
			if (wl_display_dispatch(display) == -1) {
				fprintf(stderr, "Wayland dispatch error: %s\n",
				        strerror(wl_display_get_error(display)));
				break;
			}
		}
		if (fds[1].revents & POLLIN) {
			uint8_t discard;
			time_t raw_time;
			struct tm *local_time;
			int nbytes;

			while ((nbytes = read(pfd[0], &discard, 1)) > 0);
			raw_time = time(NULL);
			local_time = localtime(&raw_time);
			strftime(clock_text, sizeof(clock_text), "%A %T %F", local_time);
			update_text_item_data(&clock_data);
		}

		if (need_draw) {
			wl_list_for_each (screen, &screens, link)
				draw(&screen->status_bar);
			need_draw = false;
		}

		wl_display_flush(display);
	}
}

int
main(int argc, char *argv[])
{
	setup();
	run();

	return EXIT_SUCCESS;
}
