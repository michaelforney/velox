/* velox: velox/layout.c
 *
 * Copyright (c) 2009 Michael Forney <mforney@mforney.org>
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
#include "resource.h"
#include "debug.h"

uint32_t layout_resource_id;

void setup_layouts()
{
    layout_resource_id = resource_type_id(LAYOUT_RESOURCE_NAME);
}

void add_layout(struct velox_layout * layout)
{
    add_resource(layout_resource_id, layout);
}

struct velox_layout * find_layout(const char * identifier)
{
    struct velox_layout ** layout;
    const struct velox_vector * layouts = resource_type_resources(layout_resource_id);

    vector_for_each(layouts, layout)
    {
        if (strcmp((*layout)->identifier, identifier) == 0)
            return *layout;
    }

    return NULL;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

