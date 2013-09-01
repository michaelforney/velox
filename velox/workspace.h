/* velox: velox/workspace.h
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

#ifndef VELOX_WORKSPACE_H
#define VELOX_WORKSPACE_H

#include <velox/layer.h>
#include <velox/layout.h>
#include <velox/list.h>
#include <velox/vector.h>

struct velox_workspace
{
    /* Might be needed with windows on multiple workspaces at once */
    // uint64_t id;
    char * name;

    struct velox_list layers;
    struct velox_window * focus;
};

extern struct velox_vector workspaces;

static inline struct velox_workspace * workspace_at(uint32_t index)
{
    return (struct velox_workspace *) vector_at(&workspaces, index);
}

void add_workspace(const char * name, struct velox_layer * layers[],
                   uint32_t num_layers);
void setup_workspaces();
void cleanup_workspaces();

void workspace_add_window(struct velox_workspace * workspace,
                          struct velox_window * window);
void workspace_remove_window(struct velox_workspace * workspace,
                             struct velox_window * window);
void workspace_set_focus(struct velox_workspace * workspace,
                         struct velox_window * window);
struct velox_layer * workspace_find_layer(struct velox_workspace * workspace,
                                          layer_predicate_t pred);
void workspace_show(struct velox_workspace * workspace);
void workspace_hide(struct velox_workspace * workspace);
void workspace_focus_next(struct velox_workspace * workspace);
void workspace_focus_prev(struct velox_workspace * workspace);
void workspace_swap_next(struct velox_workspace * workspace);
void workspace_swap_prev(struct velox_workspace * workspace);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

