/* velox: velox/tag.h
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

#ifndef VELOX_TAG_H
#define VELOX_TAG_H

#include <xcb/xcb.h>

#include <velox/layout.h>
#include <velox/list.h>
#include <velox/vector.h>

enum velox_tag_focus_type
{
    TILE,
    FLOAT
};

struct velox_tag
{
    /* Might be needed with windows on multiple tags at once */
    // uint64_t id;
    char * name;

    struct
    {
        struct list_head windows;
        struct list_head * focus;
    } tiled;

    struct
    {
        struct velox_window * top;
        struct list_head windows;
        struct list_head * next_focus;
    } floated;

    enum velox_tag_focus_type focus_type;

    struct list_head layouts;
    struct list_head * layout;
    struct velox_layout_state state;
};

DEFINE_VECTOR(velox_tag_vector, struct velox_tag *);

extern struct velox_tag_vector tags;

void setup_tags();
void cleanup_tags();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

