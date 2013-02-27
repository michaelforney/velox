/* velox: velox/workspace.c
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

void add_workspace(const char * name, const char * layout_names[])
{
    struct velox_workspace * workspace;
    struct velox_layout_entry * entry;
    char binding_name[128];

    /* Allocate a new workspace, then set its attributes */
    workspace = vector_add(&workspaces);

    /* Might be needed with windows on multiple workspaces at once */
    // workspace->id = 1 << workspace_count++;
    workspace->name = strdup(name);

    list_init(&workspace->tiled.windows);
    workspace->tiled.focus = &workspace->tiled.windows.head;

    list_init(&workspace->floated.windows);

    workspace->focus_type = TILE;

    list_init(&workspace->layouts);
    for (; *layout_names != NULL; ++layout_names)
    {
        entry = (struct velox_layout_entry *) malloc(sizeof(struct velox_layout_entry));
        entry->layout = find_layout(*layout_names);
        list_append(&workspace->layouts, entry);
    }

    workspace->layout = list_first_link(&workspace->layouts);
    workspace->state = link_entry(workspace->layout, struct velox_layout_entry)
        ->layout->default_state;

    sprintf(binding_name, "set_workspace_%u", workspaces.size);
    add_key_binding("workspace", binding_name, &set_workspace, uint32_argument(workspaces.size - 1));
    sprintf(binding_name, "move_focus_to_workspace_%u", workspaces.size);
    add_key_binding("workspace", binding_name, &move_focus_to_workspace, uint32_argument(workspaces.size - 1));
}

void setup_workspaces()
{
    struct velox_workspace * workspace;
    const char * default_layouts[] = {
        "tile",
        "grid",
        NULL
    };
    struct velox_layout_entry * entry;

    /* TODO: Make this configurable */

    add_workspace("term",     default_layouts);
    add_workspace("www",      default_layouts);
    add_workspace("irc",      default_layouts);
    add_workspace("im",       default_layouts);
    add_workspace("code",     default_layouts);
    add_workspace("mail",     default_layouts);
    add_workspace("gfx",      default_layouts);
    add_workspace("music",    default_layouts);
    add_workspace("misc",     default_layouts);
}

void cleanup_workspaces()
{
    struct velox_workspace * workspace;
    struct velox_workspace_entry * workspace_entry, * workspace_temp;
    struct velox_window_entry * window_entry;
    struct velox_layout_entry * layout_entry;
    struct velox_link * tmp;

    vector_for_each(&workspaces, workspace)
    {
        /* Free the workspace's windows */
        list_for_each_entry_safe(&workspace->tiled.windows, window_entry, tmp)
        {
            free(window_entry->window);
            free(window_entry);
        }

        /* Free the workspace's layouts */
        list_for_each_entry_safe(&workspace->layouts, layout_entry, tmp)
        {
            free(layout_entry);
        }

        free(workspace->name);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

