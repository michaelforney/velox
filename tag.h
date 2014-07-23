/* velox: tag.h
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

#ifndef VELOX_TAG_H
#define VELOX_TAG_H

#include "config.h"

#include <stdbool.h>
#include <wayland-server.h>

#define TAG_MASK(n) (1 << (n))

struct window;

struct tag
{
    char * name;
    uint32_t mask;
    struct screen * screen;
    struct wl_list link;

    struct wl_global * global;
    struct wl_list resources;

    struct
    {
        struct config_node group, name, activate, toggle, apply;
    } config;
};

void tag_add_config_nodes();

struct tag * tag_new(unsigned index, const char * name);
void tag_destroy(struct tag * tag);

/**
 * Add a tag to a screen.
 *
 * It must have been removed from its previous screen with tag_remove (even if
 * the previous screen was NULL).
 */
void tag_add(struct tag * tag, struct screen * screen);

/**
 * Remove a tag from a screen.
 */
void tag_remove(struct tag * tag, struct screen * screen);

/**
 * Set a tag's screen by removing it from the original screen, then adding it to
 * the new screen.
 *
 * This is exactly equivalent to tag_remove(tag, tag->screen) followed by
 * tag_add(tag, screen).
 */
void tag_set(struct tag * tag, struct screen * screen);

/**
 * Send a screen event for this tag.
 *
 * Either tag_resource, screen_resource, or both may be NULL in which case the
 * correct resource is found using client.
 */
void tag_send_screen(struct tag * tag, struct wl_client * client,
                     struct wl_resource * tag_resource,
                     struct wl_resource * screen_resource);

#endif

