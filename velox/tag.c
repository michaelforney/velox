/* velox: velox/tag.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of velox.
 *
 * velox is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * velox is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with velox.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libvelox/list.h>

#include "tag.h"
#include "velox.h"

#include "layout-private.h"

struct velox_list * tags;

uint8_t tag_count;

void add_tag(const char * name, struct velox_loop * layouts)
{
    struct velox_tag * tag;

    /* Allocate a new tag, then set its attributes */
    tag = (struct velox_tag *) malloc(sizeof(struct velox_tag));
    memset(tag, 0, sizeof(struct velox_tag));

    /* Might be needed with windows on multiple tags at once */
    // tag->id = 1 << tag_count++;
    tag->name = strdup(name);
    tag->layout = layouts;
    tag->state = ((struct velox_layout *) layouts->data)->default_state;

    /* Add the tag to the list of tags */
    tags = velox_list_insert(tags, tag);
}

void setup_tags()
{
    struct velox_tag * tag;
    struct velox_loop * default_layouts;

    tags = NULL;
    tag_count = 0;

    /* TODO: Make this configurable */

    default_layouts = NULL;
    default_layouts = velox_loop_insert(default_layouts, velox_hashtable_lookup(layouts, "tile"));
    default_layouts = velox_loop_insert(default_layouts, velox_hashtable_lookup(layouts, "grid"));

    add_tag("term",     velox_loop_copy(default_layouts));
    add_tag("www",      velox_loop_copy(default_layouts));
    add_tag("irc",      velox_loop_copy(default_layouts));
    add_tag("im",       velox_loop_copy(default_layouts));
    add_tag("code",     velox_loop_copy(default_layouts));
    add_tag("mail",     velox_loop_copy(default_layouts));
    add_tag("gfx",      velox_loop_copy(default_layouts));
    add_tag("music",    velox_loop_copy(default_layouts));
    add_tag("misc",     velox_loop_copy(default_layouts));

    velox_loop_delete(default_layouts, false);

    tags = velox_list_reverse(tags);
}

void cleanup_tags()
{
    while (tags != NULL)
    {
        /* Delete the layout loop structure, without freeing the data. We let
         * the layout cleanup function do this */
        velox_loop_delete(((struct velox_tag *) tags->data)->layout, false);

        /* Free the tag, then remove it from the list */
        free(tags->data);
        tags = velox_list_remove_first(tags);
    }
}

#define TAG_FUNCTIONS(N) \
    void set_tag_ ## N() \
    { \
        set_tag(N); \
    } \
    \
    void move_focus_to_tag_ ## N() \
    { \
        move_focus_to_tag(N); \
    }

/* TODO: These can now all be a single function thanks to arguments to
 * keybindings */
TAG_FUNCTIONS(1)
TAG_FUNCTIONS(2)
TAG_FUNCTIONS(3)
TAG_FUNCTIONS(4)
TAG_FUNCTIONS(5)
TAG_FUNCTIONS(6)
TAG_FUNCTIONS(7)
TAG_FUNCTIONS(8)
TAG_FUNCTIONS(9)

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

