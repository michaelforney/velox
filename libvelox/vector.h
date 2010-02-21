/* velox: libvelox/vector.c
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
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

#ifndef LIBVELOX_VECTOR
#define LIBVELOX_VECTOR

#include <stdint.h>
#include <stdbool.h>

struct velox_vector32
{
    uint32_t * data;
    uint32_t size;
    uint32_t capacity;
};

/**
 * Create a new vector of 32 bit integers
 *
 * @param capacity The capacity of the new vector. This will be rounded up to
 * the next power of two
 * @return The new vector
 */
struct velox_vector32 * velox_vector32_create(uint32_t capacity);

/**
 * Increase the capacity of a vector. The capacity will be doubled.
 *
 * @param vector The vector to increase the capacity of
 */
void velox_vector32_increase_capacity(struct velox_vector32 * vector);

/**
 * Insert a new value at the beginning of a vector
 *
 * @param vector The vector in which to insert the value
 * @param data The value to insert
 */
void velox_vector32_insert(struct velox_vector32 * vector, uint32_t data);

/**
 * Append a new value at the end of a vector
 *
 * @param vector The vector in which to append the value
 * @param data The value to append
 */
void velox_vector32_append(struct velox_vector32 * vector, uint32_t data);

/**
 * Remove a value at the specified position in a vector
 *
 * @param vector The vector from which to remove the value
 * @param index The index of the value to remove
 */
void velox_vector32_remove_at(struct velox_vector32 * vector, uint32_t index);

/**
 * Remove a value from a vector. The first occurance will be removed
 *
 * @param vector The vector from which to remove the value
 * @param data The value to remove
 */
void velox_vector32_remove(struct velox_vector32 * vector, uint32_t data);

/**
 * Reverse the contents of a vector
 *
 * @param vector The vector to reverse
 */
void velox_vector32_reverse(struct velox_vector32 * vector);

/**
 * Delete a vector, freeing its data and itself
 *
 * @param vector The vector to delete
 */
void velox_vector32_delete(struct velox_vector32 * vector);

/**
 * Clear the contents of a vector
 *
 * @param vector The vector to clear
 */
void velox_vector32_clear(struct velox_vector32 * vector);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

