/* velox: velox/manage.c
 *
 * Copyright (c) 2013 Michael Forney <mforney@mforney.org>
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

#include "debug.h"
#include "hook.h"
#include "manage.h"
#include "velox.h"
#include "window.h"

#include "hook-private.h"
#include "operations-private.h"

void manage(struct velox_window * window)
{
    DEBUG_ENTER

    if (!window->workspace)
        window->workspace = active_workspace;

    if (!window->layer)
    {
        window->layer = list_first(&window->workspace->layers,
                                   struct velox_layer);
    }

    workspace_add_window(window->workspace, window);

    if (window->workspace == active_workspace)
    {
        struct velox_layer * layer;

        list_for_each_entry(&active_workspace->layers, layer)
            layer_update(layer);

        show_window(window);
        focus(window);
        run_hooks(window, VELOX_HOOK_MANAGE_POST);
    }
}

void unmanage(struct velox_window * window)
{
    DEBUG_ENTER

    workspace_remove_window(window->workspace, window);
    commit_focus();
    run_hooks(window, VELOX_HOOK_UNMANAGE);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

