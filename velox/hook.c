/* velox: velox/hook.c
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

#include "velox.h"
#include "hook.h"

const uint16_t velox_hook_types = 7;

/* Manage hooks */
void handle_floating(struct velox_window * window);
void handle_fullscreen(struct velox_window * window);

struct velox_list * startup_hooks;
struct velox_list * manage_hooks;

struct velox_list ** hooks;

void setup_hooks()
{
    hooks = (struct velox_list **) calloc(velox_hook_types, sizeof(struct velox_list *));

    // TODO: Should these be a part of some plugin instead?
    add_hook((velox_hook_t) &handle_floating, VELOX_HOOK_MANAGE_PRE);
    add_hook((velox_hook_t) &handle_fullscreen, VELOX_HOOK_MANAGE_PRE);
}

void cleanup_hooks()
{
    uint16_t index = 0;

    if (hooks != NULL)
    {
        for (index = 0; index < velox_hook_types; ++index)
        {
            velox_list_delete(hooks[index], false);
        }

        free(hooks);
    }
}

void add_hook(velox_hook_t hook, enum velox_hook_type type)
{
    hooks[type] = velox_list_insert(hooks[type], hook);
}

void run_hooks(void * arg, enum velox_hook_type type)
{
    struct velox_list * iterator;

    for (iterator = hooks[type]; iterator != NULL; iterator = iterator->next)
    {
        ((velox_hook_t) iterator->data)(arg);
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

