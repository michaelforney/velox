/* velox: velox/layout.c
 *
 * Copyright (c) 2009 Michael Forney <michael@obberon.com>
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "velox.h"
#include "window.h"
#include "layout.h"

#include "layout-private.h"

struct velox_hashtable * layouts;

void setup_layouts()
{
    /* Create a new hashtable to store the layouts */
    layouts = velox_hashtable_create(1024, &sdbm_hash);
}

void cleanup_layouts()
{
    /* Delete the hashtable, and free all of the layouts */
    velox_hashtable_delete(layouts, true);
}

void add_layout(const char const * identifier, velox_arrange_t arrange, struct velox_layout_state * default_state)
{
    struct velox_layout * layout;

    /* Create a new layout and set its fields */
    layout = (struct velox_layout *) malloc(sizeof(struct velox_layout));

    layout->identifier = strdup(identifier);
    layout->arrange = arrange;
    layout->default_state = *default_state;

    /* Make sure the layout doesn't already exist in the table
     * TODO: Handle collisions */
    assert(!velox_hashtable_exists(layouts, layout->identifier));
    velox_hashtable_insert(layouts, layout->identifier, layout);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

