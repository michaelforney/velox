/* velox: velox/vector.h
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

#ifndef VELOX_VECTOR
#define VELOX_VECTOR

#include <stdint.h>
#include <stdbool.h>

#define DEFINE_VECTOR(type, data_type)                                      \
    struct type                                                             \
    {                                                                       \
        data_type * data;                                                   \
        uint32_t size;                                                      \
        uint32_t capacity;                                                  \
    }

#define VECTOR_INIT() { NULL, 0, 0 }                                        \

#define VECTOR(type, name)                                                  \
    struct type name = VECTOR_INIT()

static uint32_t next_power_of_two(uint32_t n)
{
    uint8_t bit;

    --n;
    for (bit = 1; bit < 32; bit <<= 1, n |= n >> bit);

    return ++n;
}

/**
 * Initialize a new vector with the specified capacity
 *
 * @param capacity The capacity of the new vector. This will be rounded up to
 * the next power of two
 */
#define vector_initialize(vector, initial_capacity)                         \
{                                                                           \
    (vector)->size = 0;                                                     \
    (vector)->capacity = next_power_of_two(initial_capacity);               \
    (vector)->data = (typeof((vector)->data))                               \
        malloc((vector)->capacity * sizeof(typeof(*(vector)->data)));       \
}

/**
 * Increase the capacity of a vector. The capacity will be doubled.
 *
 * @param vector The vector to increase the capacity of
 */
#define vector_increase_capacity(vector)                                    \
{                                                                           \
    (vector)->data = (typeof((vector)->data)) realloc((vector)->data,       \
        2 * (vector)->capacity * sizeof(typeof(*(vector)->data)));          \
    memset((vector)->data + (vector)->capacity, 0,                          \
        (vector)->capacity * sizeof(typeof(*(vector)->data)));              \
    (vector)->capacity *= 2;                                                \
}

/**
 * Insert a new value at the beginning of a vector
 *
 * @param vector The vector in which to insert the value
 * @param data The value to insert
 */
#define vector_insert(vector, value)                                        \
{                                                                           \
    if ((vector)->size == (vector)->capacity)                               \
        vector_increase_capacity(vector);                                   \
    memmove((vector)->data + 1, (vector)->data, (vector)->size);            \
    (vector)->data[0] = value;                                              \
    ++(vector)->size;                                                       \
}

/**
 * Append a new value at the end of a vector
 *
 * @param vector The vector in which to append the value
 * @param data The value to append
 */
#define vector_append(vector, value)                                        \
{                                                                           \
    if ((vector)->size == (vector)->capacity)                               \
        vector_increase_capacity(vector);                                   \
    (vector)->data[(vector)->size++] = value;                               \
}

/**
 * Remove a value at the specified position in a vector
 *
 * @param vector The vector from which to remove the value
 * @param index The location of the value to remove
 */
#define vector_remove_at(vector, position)                                  \
{                                                                           \
    memmove(position, position + 1,                                         \
        ((vector)->size - (position - (vector)->data) - 1) *                \
            sizeof(typeof(*(vector)->data)));                               \
    --(vector)->size;                                                       \
}

/**
 * Traverse the contents of a vector.
 *
 * @param vector The vector to traverse
 * @param position The value to use as the iterator
 */
#define vector_for_each(vector, position)                                   \
    for (position = (vector)->data;                                         \
        position - (vector)->data < (vector)->size;                         \
        ++position)                                                         \

/**
 * Free the contents of a vector
 *
 * @param vector The vector to free
 */
#define vector_free(vector) free((vector)->data)

/**
 * Clear the contents of a vector
 *
 * @param vector The vector to clear
 */
#define vector_clear(vector) (vector)->size = 0

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

