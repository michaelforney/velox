/* velox: velox.c
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

#include "velox.h"
#include "config.h"
#include "layout.h"
#include "screen.h"
#include "tag.h"
#include "window.h"
#include "protocol/velox-server-protocol.h"

#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <swc.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>

struct velox velox;
unsigned border_width = 2;

static void
new_screen(struct swc_screen *swc)
{
	struct screen *screen;

	if (!(screen = screen_new(swc)))
		return;

	velox.active_screen = screen;
	wl_list_insert(&velox.screens, &screen->link);
}

static void
new_window(struct swc_window *swc)
{
	struct window *window;

	if (!(window = window_new(swc)))
		return;

	manage(window);
}

const struct swc_manager manager = {
	.new_screen = &new_screen,
	.new_window = &new_window,
};

static void
get_screen(struct wl_client *client, struct wl_resource *resource,
           struct wl_resource *screen_resource, uint32_t id)
{
	struct screen *screen;
	struct swc_screen *swc_screen = wl_resource_get_user_data(screen_resource);

	wl_list_for_each (screen, &velox.screens, link) {
		if (screen->swc == swc_screen)
			goto found;
	}

	wl_resource_post_error(resource, VELOX_ERROR_INVALID_SCREEN, "Invalid screen resource");
	return;

found:
	if (!screen_bind(screen, client, id))
		wl_client_post_no_memory(client);
}

static const struct velox_interface velox_implementation = {
	.get_screen = &get_screen,
};

static void
apply_rules(struct window *window)
{
	struct rule *rule;
	const char *identifier;

	wl_list_for_each (rule, &velox.rules, link) {
		switch (rule->type) {
		case RULE_TYPE_WINDOW_TITLE:
			identifier = window->swc->title;
			break;
		case RULE_TYPE_APP_ID:
			identifier = window->swc->app_id;
			break;
		default:
			identifier = NULL;
			break;
		}

		if (!identifier)
			continue;

		if (strcmp(identifier, rule->identifier) == 0) {
			struct config_node *node = rule->action;
			const struct variant v = {
				.type = VARIANT_WINDOW,
				.window = window
			};

			node->action.run(node, &v);
		}
	}
}

void
manage(struct window *window)
{
	struct tag *tag;

	wl_list_insert(&velox.hidden_windows, &window->link);

	apply_rules(window);
	if (!window->tag) {
		tag = wl_container_of(velox.active_screen->tags.next, tag, link);
		window_set_tag(window, tag);
	}
	if (window->tag->screen)
		screen_set_focus(window->tag->screen, window);
	update();
}

void
unmanage(struct window *window)
{
	struct screen *screen = window->tag->screen;

	window_set_tag(window, NULL);
	wl_list_remove(&window->link);
	if (screen)
		screen_arrange(screen);
}

void
arrange(void)
{
	struct screen *screen;

	wl_list_for_each (screen, &velox.screens, link)
		screen_arrange(screen);
}

void
update(void)
{
	struct screen *screen;
	struct window *window;

	/* Arrange the windows first so that they aren't shown before they are the
	 * correct size. */
	arrange();

	wl_list_for_each (screen, &velox.screens, link) {
		wl_list_for_each (window, &screen->windows, link)
			window_show(window);
	}

	wl_list_for_each (window, &velox.hidden_windows, link)
		window_hide(window);
}

struct tag *
next_tag(uint32_t *tags)
{
	unsigned index = ffs(*tags);
	struct tag *tag;

	if (index == 0)
		return NULL;

	tag = velox.tags[index - 1];
	*tags &= ~tag->mask;

	return tag;
}

struct tag *
find_unused_tag(void)
{
	struct tag *tag;

	if (wl_list_empty(&velox.unused_tags))
		return NULL;

	tag = wl_container_of(velox.unused_tags.next, tag, link);
	return tag;
}

/**** Actions ****/
static void
focus_next(struct config_node *node, const struct variant *v)
{
	screen_focus_next(velox.active_screen);
}

static void
focus_prev(struct config_node *node, const struct variant *v)
{
	screen_focus_prev(velox.active_screen);
}

static void
zoom(struct config_node *node, const struct variant *v)
{
	struct screen *screen = velox.active_screen;
	struct wl_list *link;

	if (!screen->focus)
		return;

	/* Move the focus to the beginning of the window list, or if it is already
	 * there, the window after the focus. */
	link = &screen->focus->link;

	if (screen->windows.next == link) {
		if (link->next == &screen->windows)
			return;

		link = link->next;
	}

	wl_list_remove(link);
	wl_list_insert(&screen->windows, link);
	arrange();
}

static void
layout_next(struct config_node *node, const struct variant *v)
{
	struct screen *screen = velox.active_screen;
	struct layout **layout = &screen->layout[TILE];
	struct wl_list *link;

	if ((link = (*layout)->link.next) == &screen->layouts)
		link = link->next;
	*layout = wl_container_of(link, *layout, link);
	screen_arrange(screen);
}

static void
previous_tags(struct config_node *node, const struct variant *v)
{
	uint32_t mask = velox.active_screen->last_mask;

	velox.active_screen->last_mask = velox.active_screen->mask;
	screen_set_tags(velox.active_screen, mask);
	update();
}

static void
quit(struct config_node *node, const struct variant *v)
{
	wl_display_terminate(velox.display);
}

static CONFIG_ACTION(focus_next, &focus_next);
static CONFIG_ACTION(focus_prev, &focus_prev);
static CONFIG_ACTION(zoom, &zoom);
static CONFIG_ACTION(layout_next, &layout_next);
static CONFIG_ACTION(previous_tags, &previous_tags);
static CONFIG_ACTION(quit, &quit);

static void
add_config_nodes(void)
{
	wl_list_insert(config_root, &focus_next_action.link);
	wl_list_insert(config_root, &focus_prev_action.link);
	wl_list_insert(config_root, &zoom_action.link);
	wl_list_insert(config_root, &layout_next_action.link);
	wl_list_insert(config_root, &previous_tags_action.link);
	wl_list_insert(config_root, &quit_action.link);

	layout_add_config_nodes();
	tag_add_config_nodes();
	window_add_config_nodes();
}

static void
start_clients(void)
{
	char path[PATH_MAX];
	const char *dir;
	int ret;

	if (!(dir = getenv("VELOX_LIBEXEC")))
		dir = VELOX_LIBEXEC;

	ret = snprintf(path, sizeof(path), "%s/status_bar", dir);
	if (ret < 0 || ret >= sizeof(path))
		return;
	if (fork() == 0) {
		execl(path, path, NULL);
		exit(EXIT_FAILURE);
	}
}

static int
handle_chld(int num, void *data)
{
	/* Clean up zombie processes. */
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	return 0;
}

static void
bind_velox(struct wl_client *client, void *data,
           uint32_t version, uint32_t id)
{
	struct wl_resource *resource;

	if (version >= 1)
		version = 1;

	if (!(resource = wl_resource_create(client, &velox_interface, version, id))) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource, &velox_implementation, NULL, NULL);
}

int
main(int argc, char *argv[])
{
	const char *socket;
	int index;
	char tag_name[] = "1";

	velox.display = wl_display_create();
	if (!velox.display)
		goto error0;

	socket = wl_display_add_socket_auto(velox.display);
	if (!socket)
		goto error1;
	setenv("WAYLAND_DISPLAY", socket, 1);

	velox.global = wl_global_create(velox.display, &velox_interface, 1, NULL, &bind_velox);
	if (!velox.global)
		goto error1;

	velox.event_loop = wl_display_get_event_loop(velox.display);
	wl_event_loop_add_signal(velox.event_loop, SIGCHLD, &handle_chld, NULL);
	wl_list_init(&velox.screens);
	wl_list_init(&velox.hidden_windows);
	wl_list_init(&velox.unused_tags);
	wl_list_init(&velox.rules);
	add_config_nodes();

	for (index = 0; index < NUM_TAGS; ++index, ++tag_name[0]) {
		if (!(velox.tags[index] = tag_new(index, tag_name)))
			goto error2;
	}

	/* Mark tags as unused in reverse order, so that they are claimed in ascending
	 * order. */
	for (index = NUM_TAGS - 1; index >= 0; --index)
		tag_add(velox.tags[index], NULL);

	if (!swc_initialize(velox.display, NULL, &manager))
		goto error2;

	if (!config_parse())
		goto error3;

	start_clients();

	wl_display_run(velox.display);
	swc_finalize();

	return EXIT_SUCCESS;

error3:
	swc_finalize();
error2:
	while (index > 0)
		tag_destroy(velox.tags[--index]);
	wl_global_destroy(velox.global);
error1:
	wl_display_destroy(velox.display);
error0:
	return EXIT_FAILURE;
}
