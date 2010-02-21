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

#include <stdlib.h>
#include <string.h>

#include "vector.h"

uint32_t next_power_of_two(uint32_t n)
{
    uint8_t bit;

    --n;
    for (bit = 1; bit < 32; bit <<= 1, n |= n >> bit);

    return ++n;
}

struct velox_vector32 * velox_vector32_create(uint32_t capacity)
{
    struct velox_vector32 * vector;

    vector = (struct velox_vector32 *) malloc(sizeof(struct velox_vector32 *));
    vector->size = 0;
    vector->capacity = next_power_of_two(capacity);
    vector->data = calloc(vector->capacity, sizeof(uint32_t));

    return vector;
}

void velox_vector32_increase_capacity(struct velox_vector32 * vector)
{
    vector->data = realloc(vector->data, 2 * vector->capacity * sizeof(uint32_t));
    memset(vector->data + vector->capacity, 0, vector->capacity * sizeof(uint32_t));
    vector->capacity *= 2;
}

void velox_vector32_insert(struct velox_vector32 * vector, uint32_t data)
{
    if (vector->size == vector->capacity)
    {
        velox_vector32_increase_capacity(vector);
    }

    memmove(vector->data + 1, vector->data, vector->size);
    vector->data[0] = data;
    ++vector->size;
}

void velox_vector32_append(struct velox_vector32 * vector, uint32_t data)
{
    if (vector->size == vector->capacity)
    {
        velox_vector32_increase_capacity(vector);
    }

    vector->data[vector->size++] = data;
}

void velox_vector32_remove_at(struct velox_vector32 * vector, uint32_t index)
{
    assert(index < vector->size);

    memmove(vector->data + index, vector->data + index + 1, (vector->size - index - 1) * 4);
    vector->data[--vector->size] = 0;
}

void velox_vector32_remove(struct velox_vector32 * vector, uint32_t data)
{
    uint32_t index;

    for (index = 0; index < vector->size; ++index)
    {
        if (vector->data[index] == data)
        {
            velox_vector32_remove_at(vector, index);
            return;
        }
    }
}

void velox_vector32_reverse(struct velox_vector32 * vector)
{
    uint32_t * first, * second;
    uint32_t temp;

    for (first = vector->data, second = vector->data + vector->size - 1;
        first < second;
        ++first, --second)
    {
        temp = *first;
        *first = *second;
        *second = temp;
    }
}

void velox_vector32_delete(struct velox_vector32 * vector)
{
    free(vector->data);
    free(vector);
}

void velox_vector32_clear(struct velox_vector32 * vector)
{
    memset(vector->data, 0, vector->size * 4);
    vector->size = 0;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

