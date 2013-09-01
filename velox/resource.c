/* velox: velox/resource.c
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

#include <velox/vector.h>

#include "resource.h"

struct resource_type
{
    const char * name;
    void (* destroy)(void *);
    struct velox_vector resources;
};

struct velox_vector resource_types;

static void __attribute__((constructor)) initialize_resources()
{
    vector_initialize(&resource_types, sizeof(void *), 32);
}

static void __attribute__((destructor)) free_resources()
{
    vector_free(&resource_types);
}

uint32_t resource_type_id(const char * const name)
{
    struct resource_type * type;
    uint32_t index;

    vector_for_each_with_index(&resource_types, type, index)
    {
        if (strcmp(name, type->name) == 0)
            return index;
    }

    type = vector_add(&resource_types);
    type->name = strdup(name);
    type->destroy = NULL;
    vector_initialize(&type->resources, sizeof(void *), 32);

    return index;
}

void resource_type_set_destroy(uint32_t type_id, void (* destroy)(void *))
{
    struct resource_type * type;

    type = vector_at(&resource_types, type_id);
    type->destroy = destroy;
}

const struct velox_vector * resource_type_resources(uint32_t type_id)
{
    struct resource_type * type;

    type = vector_at(&resource_types, type_id);
    return &type->resources;
}

void add_resource(uint32_t type_id, void * resource)
{
    struct resource_type * type;

    type = vector_at(&resource_types, type_id);
    vector_add_value(&type->resources, resource);
}

void cleanup_resources()
{
    struct resource_type * type;
    void * resource;

    vector_for_each(&resource_types, type)
    {
        vector_for_each(&type->resources, resource)
        {
            if (type->destroy)
                type->destroy(resource);
        }

        vector_free(&type->resources);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

