/* velox: velox/tag.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <mforney@mforney.org>
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

#include "tag.h"
#include "velox.h"
#include "binding.h"

#include "layout-private.h"

struct velox_vector tags;

static void __attribute__((constructor)) initialize_tags()
{
    vector_initialize(&tags, sizeof(struct velox_tag), 32);
}

static void __attribute__((destructor)) free_tags()
{
    vector_free(&tags);
}

void add_tag(const char * name, const char * layout_names[])
{
    struct velox_tag * tag;
    struct velox_layout_entry * layout_entry;
    char binding_name[128];

    /* Allocate a new tag, then set its attributes */
    tag = vector_add(&tags);

    /* Might be needed with windows on multiple tags at once */
    // tag->id = 1 << tag_count++;
    tag->name = strdup(name);

    INIT_LIST_HEAD(&tag->tiled.windows);
    tag->tiled.focus = &tag->tiled.windows;

    INIT_LIST_HEAD(&tag->floated.windows);

    tag->focus_type = TILE;

    INIT_LIST_HEAD(&tag->layouts);
    for (; *layout_names != NULL; ++layout_names)
    {
        layout_entry = (struct velox_layout_entry *) malloc(sizeof(struct velox_layout_entry));
        layout_entry->layout = hashtable_lookup(&layouts, *layout_names);
        list_add_tail(&layout_entry->head, &tag->layouts);
    }

    tag->layout = tag->layouts.next;
    tag->state = list_entry(tag->layout, struct velox_layout_entry, head)->layout->default_state;

    sprintf(binding_name, "set_tag_%u", tags.size);
    add_key_binding("tag", binding_name, &set_tag, uint32_argument(tags.size - 1));
    sprintf(binding_name, "move_focus_to_tag_%u", tags.size);
    add_key_binding("tag", binding_name, &move_focus_to_tag, uint32_argument(tags.size - 1));
}

void setup_tags()
{
    struct velox_tag * tag;
    const char * default_layouts[] = {
        "tile",
        "grid",
        NULL
    };
    struct velox_layout_entry * entry;

    /* TODO: Make this configurable */

    add_tag("term",     default_layouts);
    add_tag("www",      default_layouts);
    add_tag("irc",      default_layouts);
    add_tag("im",       default_layouts);
    add_tag("code",     default_layouts);
    add_tag("mail",     default_layouts);
    add_tag("gfx",      default_layouts);
    add_tag("music",    default_layouts);
    add_tag("misc",     default_layouts);
}

void cleanup_tags()
{
    struct velox_tag * tag;
    struct velox_tag_entry * tag_entry, * tag_temp;
    struct velox_window_entry * window_entry, * window_temp;
    struct velox_layout_entry * layout_entry, * layout_temp;

    vector_for_each(&tags, tag)
    {
        /* Free the tag's windows */
        list_for_each_entry_safe(window_entry, window_temp, &tag->tiled.windows, head)
        {
            free(window_entry->window);
            free(window_entry);
        }

        /* Free the tag's layouts */
        list_for_each_entry_safe(layout_entry, layout_temp, &tag->layouts, head)
        {
            free(layout_entry);
        }

        free(tag->name);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

