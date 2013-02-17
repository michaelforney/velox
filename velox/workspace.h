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

#include <xcb/xcb.h>

#include <velox/layout.h>
#include <velox/list.h>
#include <velox/vector.h>

enum velox_workspace_focus_type
{
    TILE,
    FLOAT
};

struct velox_workspace
{
    /* Might be needed with windows on multiple workspaces at once */
    // uint64_t id;
    char * name;

    struct
    {
        struct list_head windows;
        struct list_head * focus;
    } tiled;

    struct
    {
        struct list_head windows;
    } floated;

    enum velox_workspace_focus_type focus_type;

    struct list_head layouts;
    struct list_head * layout;
    struct velox_layout_state state;
};

extern struct velox_vector workspaces;

static inline struct velox_workspace * workspace_at(uint32_t index)
{
    return (struct velox_workspace *) vector_at(&workspaces, index);
}

void setup_workspaces();
void cleanup_workspaces();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

