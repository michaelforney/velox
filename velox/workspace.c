/* velox: velox/workspace.c
 *
 * Copyright (c) 2009, 2010, 2013 Michael Forney <mforney@mforney.org>
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
#include <assert.h>

#include "workspace.h"
#include "velox.h"
#include "binding.h"

struct velox_vector workspaces;

static void __attribute__((constructor)) initialize_workspaces()
{
    vector_initialize(&workspaces, sizeof(struct velox_workspace), 32);
}

static void __attribute__((destructor)) free_workspaces()
{
    vector_free(&workspaces);
}

void add_workspace(const char * name, struct velox_layer * layers[],
                   uint32_t num_layers)
{
    struct velox_workspace * workspace;
    struct velox_layout_entry * entry;
    char binding_name[64];
    uint32_t index;

    /* Allocate a new workspace, then set its attributes */
    workspace = vector_add(&workspaces);

    workspace->name = strdup(name);
    workspace->focus = NULL;

    list_init(&workspace->layers);

    for (index = 0; index < num_layers; ++index)
        list_append(&workspace->layers, layers[index]);

    sprintf(binding_name, "set_workspace_%u", workspaces.size);
    add_key_binding("workspace", binding_name, &set_workspace, uint32_argument(workspaces.size - 1));
    sprintf(binding_name, "move_focus_to_workspace_%u", workspaces.size);
    add_key_binding("workspace", binding_name, &move_focus_to_workspace, uint32_argument(workspaces.size - 1));
}

void setup_workspaces()
{
}

void cleanup_workspaces()
{
    struct velox_workspace * workspace;
    struct velox_layer * layer;
    struct velox_link * tmp;

    vector_for_each(&workspaces, workspace)
    {
        /* Free the workspace's layers */
        list_for_each_entry_safe(&workspace->layers, layer, tmp)
            layer_destroy(layer);

        free(workspace->name);
    }
}

void workspace_add_window(struct velox_workspace * workspace,
                          struct velox_window * window)
{
    layer_add_window(window->layer, window);
    workspace->focus = window->layer->focus;
    window->workspace = workspace;
}

void workspace_remove_window(struct velox_workspace * workspace,
                             struct velox_window * window)
{
    struct velox_layer * layer;
    struct velox_window * focus;

    layer_remove_window(window->layer, window);

    if (workspace->focus == window)
    {
        /* If the layer is now empty, choose a new layer. */
        if (!(workspace->focus = window->layer->focus))
        {
            list_for_each_entry(&workspace->layers, layer)
            {
                if ((workspace->focus = layer->focus))
                    break;
            }
        }
    }

    if (workspace == active_workspace)
        layer_update(window->layer);

    window->layer = NULL;
}

void workspace_set_focus(struct velox_workspace * workspace,
                         struct velox_window * window)
{
    if (workspace->focus == window)
        return;

    layer_set_focus(window->layer, window);
    workspace->focus = window;
}

struct velox_layer * workspace_find_layer(struct velox_workspace * workspace,
                                          layer_predicate_t pred)
{
    struct velox_layer * layer;

    list_for_each_entry(&workspace->layers, layer)
    {
        if (pred(layer))
            return layer;
    }

    return NULL;
}

void workspace_focus_next(struct velox_workspace * workspace)
{
    struct velox_layer * layer;

    layer = workspace->focus->layer;
    layer_focus_next(layer);
    workspace->focus = layer->focus;
}

void workspace_focus_prev(struct velox_workspace * workspace)
{
    struct velox_layer * layer;

    layer = workspace->focus->layer;
    layer_focus_prev(layer);
    workspace->focus = layer->focus;
}

void workspace_swap_next(struct velox_workspace * workspace)
{
    layer_swap_next(workspace->focus->layer);
}

void workspace_swap_prev(struct velox_workspace * workspace)
{
    layer_swap_prev(workspace->focus->layer);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

