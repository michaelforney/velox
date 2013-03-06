/* velox: velox/resource.h
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

#ifndef VELOX_RESOURCE_H
#define VELOX_RESOURCE_H

#include <stdint.h>
#include <velox/list.h>

/**
 * Obtains a resource type ID from its name and size.
 *
 * @param name The name of the resource type.
 */
uint32_t resource_type_id(const char * const name);

/**
 * Sets the method used to destroy resources of a particular type.
 */
void resource_type_set_destroy(uint32_t type_id, void (* destroy)(void *));

/**
 * Gets a list of resources of a particular type.
 */
const struct velox_vector * resource_type_resources(uint32_t type_id);

void add_resource(uint32_t type_id, void * resource);

void cleanup_resources();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

