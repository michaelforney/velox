/* mwm: mwm/hook.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of mwm.
 *
 * mwm is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * mwm is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with mwm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mwm.h"
#include "hook.h"

/* Manage hooks */
void handle_floating(struct mwm_window * window);
void handle_fullscreen(struct mwm_window * window);

struct mwm_list * startup_hooks;
struct mwm_list * manage_hooks;

void setup_hooks()
{
    add_manage_hook(&handle_floating);
    add_manage_hook(&handle_fullscreen);
}

void add_startup_hook(mwm_startup_hook_t hook)
{
    startup_hooks = mwm_list_insert(startup_hooks, hook);
}

void add_manage_hook(mwm_manage_hook_t hook)
{
    manage_hooks = mwm_list_insert(manage_hooks, hook);
}

void run_startup_hooks()
{
    struct mwm_list * iterator;

    for (iterator = startup_hooks; iterator != NULL; iterator = iterator->next)
    {
        ((mwm_startup_hook_t) iterator->data)();
    }
}

void run_manage_hooks(struct mwm_window * window)
{
    struct mwm_list * iterator;

    for (iterator = manage_hooks; iterator != NULL; iterator = iterator->next)
    {
        ((mwm_manage_hook_t) iterator->data)(window);
    }
}

/* Manage hooks */
void handle_floating(struct mwm_window * window)
{
    if (strcmp(window->name, "MPlayer") == 0)
    {
        window->floating = true;
    }
}

void handle_fullscreen(struct mwm_window * window)
{
    if (window->width == screen_width && window->height == screen_height)
    {
        printf("fullscreen window\n");

        window->x = 0;
        window->y = 0;
        window->border_width = 0;
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

