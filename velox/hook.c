/* velox: velox/hook.c
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

#include "velox.h"
#include "hook.h"

const uint16_t velox_hook_types = 8;

/* Manage hooks */
void handle_floating(struct velox_window * window);
void handle_fullscreen(struct velox_window * window);

struct list_head * hooks;

void setup_hooks()
{
    uint32_t index;

    hooks = (struct list_head *) malloc(velox_hook_types * sizeof(struct list_head));

    for (index = 0; index < velox_hook_types; ++index)
    {
        INIT_LIST_HEAD(&hooks[index]);
    }

    // TODO: Should these be a part of some plugin instead?
    add_hook((velox_hook_t) &handle_floating, VELOX_HOOK_MANAGE_PRE);
    add_hook((velox_hook_t) &handle_fullscreen, VELOX_HOOK_MANAGE_PRE);
}

void cleanup_hooks()
{
    uint16_t index = 0;

    if (hooks != NULL)
    {
        struct list_head * pos, * n;

        for (index = 0; index < velox_hook_types; ++index)
        {
            list_for_each_safe(pos, n, &hooks[index])
            {
                free(list_entry(pos, struct velox_hook_entry, head));
            }
        }

        free(hooks);
    }
}

void add_hook(velox_hook_t hook, enum velox_hook_type type)
{
    struct velox_hook_entry * entry;

    entry = (struct velox_hook_entry *) malloc(sizeof(struct velox_hook_entry));
    entry->hook = hook;
    list_add(&entry->head, &hooks[type]);
}

void run_hooks(void * arg, enum velox_hook_type type)
{
    struct velox_hook_entry * entry;

    list_for_each_entry(entry, &hooks[type], head)
    {
        entry->hook(arg);
    }
}

/* Manage hooks */
void handle_floating(struct velox_window * window)
{
    /* TODO: Make download konqueror windows floating */
    if (strcmp(window->name, "MPlayer") == 0)
    {
        window->floating = true;
    }
    else if (strcmp(window->name, "xclock") == 0)
    {
        window->floating = true;
    }
}

void handle_fullscreen(struct velox_window * window)
{
    if (window->width == screen_area.width && window->height == screen_area.height)
    {
        window->x = 0;
        window->y = 0;
        window->border_width = 0;
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

