/* mwm: mwm/tag.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libmwm/list.h>

#include "tag.h"
#include "mwm.h"

struct mwm_list * tags;

uint8_t tag_count;

void add_tag(const char * name, struct mwm_loop * layouts)
{
    struct mwm_tag * tag;

    tag = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    memset(tag, 0, sizeof(struct mwm_tag));

    tag->id = 1 << tag_count++;
    tag->name = strdup(name);
    tag->layout = layouts;
    tag->state = ((struct mwm_layout *) layouts->data)->default_state;

    tags = mwm_list_insert(tags, tag);
    printf("tag: %i\n", tag);
}

void setup_tags()
{
    struct mwm_tag * tag;
    struct mwm_loop * default_layouts;

    tags = NULL;
    tag_count = 0;

    default_layouts = NULL;
    default_layouts = mwm_loop_insert(default_layouts, mwm_hashtable_lookup(layouts, "tile"));
    default_layouts = mwm_loop_insert(default_layouts, mwm_hashtable_lookup(layouts, "grid"));

    add_tag("term",     mwm_loop_copy(default_layouts));
    add_tag("www",      mwm_loop_copy(default_layouts));
    add_tag("irc",      mwm_loop_copy(default_layouts));
    add_tag("im",       mwm_loop_copy(default_layouts));
    add_tag("code",     mwm_loop_copy(default_layouts));
    add_tag("mail",     mwm_loop_copy(default_layouts));
    add_tag("gfx",      mwm_loop_copy(default_layouts));
    add_tag("music",    mwm_loop_copy(default_layouts));
    add_tag("misc",     mwm_loop_copy(default_layouts));

    tags = mwm_list_reverse(tags);
}

void cleanup_tags()
{
    while (tags != NULL)
    {
        mwm_loop_delete(((struct mwm_tag *) tags->data)->layout, false);
        free(tags->data);
        tags = mwm_list_remove_first(tags);
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

TAG_FUNCTIONS(1)
TAG_FUNCTIONS(2)
TAG_FUNCTIONS(3)
TAG_FUNCTIONS(4)
TAG_FUNCTIONS(5)
TAG_FUNCTIONS(6)
TAG_FUNCTIONS(7)
TAG_FUNCTIONS(8)
TAG_FUNCTIONS(9)

