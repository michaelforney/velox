/* velox: velox/vector.h
 *
 * Copyright (c) 2010 Michael Forney <mforney@mforney.org>
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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

struct velox_vector
{
    void * data;
    uint32_t item_size;
    uint32_t size;
    uint32_t capacity;
};

static inline uint32_t next_power_of_two(uint32_t n)
{
    uint8_t bit;

    --n;
    for (bit = 1; bit < 32; bit <<= 1, n |= n >> bit);

    return ++n;
}

/**
 * Initialize a new vector with the specified capacity
 *
 * @param item_size The size of each item in the vector.
 * @param initial_capacity The initial capacity (in number of items) of the new
 * vector.
 */
static inline void vector_initialize(struct velox_vector * vector,
    uint32_t item_size, uint32_t initial_capacity)
{
    vector->size = 0;
    vector->item_size = item_size;
    vector->capacity = next_power_of_two(initial_capacity * item_size);
    vector->data = malloc(vector->capacity);
}

/**
 * Increase the capacity of a vector. The capacity will be doubled.
 *
 * @param vector The vector to increase the capacity of
 */
static inline void vector_increase_capacity(struct velox_vector * vector)
{
    vector->data = realloc(vector->data, 2 * vector->capacity
        * vector->item_size);
    vector->capacity *= 2;
}

static inline void * vector_add(struct velox_vector * vector)
{
    while (vector->size * vector->item_size > vector->capacity)
        vector_increase_capacity(vector);
    return vector->data + vector->size++ * vector->item_size;
}

/**
 * Add a new value at the end of a vector.
 *
 * @param vector The vector to append
 * @return The location of the new element
 */
#define vector_add_value(vector, value)                                     \
{                                                                           \
    *((typeof(value) *) vector_add(vector)) = value;                      \
}

/**
 * Remove a value at the specified position in a vector.
 *
 * @param vector The vector from which to remove the value
 * @param index The location of the value to remove
 */
static inline void vector_remove_at(struct velox_vector * vector,
    uint32_t index)
{
    void * position = vector->data + index * vector->item_size;
    if (index < vector->size - 1)
        memmove(position, position + vector->item_size,
            (vector->size - 1 - index) * vector->item_size);
    --vector->size;
}

/**
 * Traverse the contents of a vector.
 *
 * @param vector The vector to traverse
 * @param iterator The value to use as the iterator
 */
#define vector_for_each(vector, iterator)                                   \
    for ((iterator) = (typeof((iterator))) (vector)->data;                  \
        vector_position((vector), (iterator)) < (vector)->size;             \
        ++(iterator))                                                       \

/**
 * Traverse the contents of a vector with an index.
 *
 * @param vector The vector to traverse
 * @param iterator The value to use as the iterator
 * @param index The value to use as the index
 */
#define vector_for_each_with_index(vector, iterator, index)                 \
    for ((iterator) = (typeof(iterator)) (vector)->data, (index) = 0;       \
        (index) < (vector)->size;                                           \
        ++(index), ++(iterator))                                            \

/**
 * Returns the position of an iterator.
 *
 * @param vector The vector the iterator belongs to.
 * @param iterator The iterator of which to calculate position.
 */
static inline uint32_t vector_position(const struct velox_vector * vector,
    void * iterator)
{
    return (iterator - vector->data) / vector->item_size;
}

/**
 * Returns a pointer to the item at the specified index in the vector.
 *
 * @param vector The vector in which to index
 * @param index The index of the item to be retreived.
 */
static inline void * vector_at(struct velox_vector * vector, uint32_t index)
{
    return vector->data + index * vector->item_size;
}

/**
 * Free the contents of a vector
 *
 * @param vector The vector to free
 */
static inline void vector_free(struct velox_vector * vector)
{
    free(vector->data);
}

/**
 * Clear the contents of a vector
 *
 * @param vector The vector to clear
 */
static inline void vector_clear(struct velox_vector * vector)
{
    vector->size = 0;
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

