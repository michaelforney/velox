/* velox: libvelox/hashtable.c
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

#include "hashtable.h"

static const uint32_t hashtable_sizes[] = {
    89,
    179,
    359,
    719,
    1439,
    2879,
    5759,
    11519,
    23039,
    46079,
    92159,
    184319,
    368639,
    737279,
    1474559,
    2949119,
    5898239,
    11796479,
    23592959,
    47185919,
    94371839,
    188743679,
    377487359,
    754974719,
    1509949439
};

static const uint8_t hashtable_sizes_count = sizeof(hashtable_sizes) / 4;

uint32_t find_optimal_size(uint32_t size)
{
    uint8_t index;

    for (index = 0; size > hashtable_sizes[index] && index < hashtable_sizes_count; ++index);

    return hashtable_sizes[index];
}

/* SDBM hashing function */
uint32_t sdbm_hash(const char * string)
{
    uint32_t hash = 0;
    int32_t c;

    while ((c = *string++))
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

/**
 * Create a new hash table
 *
 * @param size The number of slots in the table
 * @param hash_function The hashing function to use
 * @return The newly created hash table
 */
struct velox_hashtable * velox_hashtable_create(uint32_t size, uint32_t (* hash_function)(const char * string))
{
    struct velox_hashtable * hashtable;
    uint32_t optimal_size;

    optimal_size = find_optimal_size(size);

    hashtable = (struct velox_hashtable *) malloc(sizeof(struct velox_hashtable));
    hashtable->data = (void **) malloc(optimal_size * sizeof(void *));
    memset(hashtable->data, 0, optimal_size * sizeof(void *));
    hashtable->hash_function = hash_function;
    hashtable->size = optimal_size;

    return hashtable;
}

/**
 * Looks up the value of a key in a hashtable
 *
 * @param hashtable The hashtable to lookup the value from
 * @param key The key to lookup
 * @return The value looked up
 */
void * velox_hashtable_lookup(struct velox_hashtable * hashtable, const char * key)
{
    return hashtable->data[hashtable->hash_function(key) % hashtable->size];
}

/**
 * Insert a value into a hashtable
 *
 * @param hashtable The hashtable to insert the value into
 * @param key The key to insert the value
 * @param value The value to insert into the hashtable
 */
void velox_hashtable_insert(struct velox_hashtable * hashtable, const char * key, void * value)
{
    hashtable->data[hashtable->hash_function(key) % hashtable->size] = value;
}

/**
 * Unset a key from a hashtable
 *
 * @param hashtable The hashtable to unset the key from
 * @param key The key to unset
 */
void velox_hashtable_unset(struct velox_hashtable * hashtable, const char * key)
{
    hashtable->data[hashtable->hash_function(key) % hashtable->size] = NULL;
}

/**
 * Check if a key already exists in a hashtable
 *
 * @param hashtable The hashtable to check for the key
 * @param key The key to check for
 * @return Whether or not the key exists in the hashtable
 */
bool velox_hashtable_exists(struct velox_hashtable * hashtable, const char * key)
{
    return hashtable->data[hashtable->hash_function(key) % hashtable->size] != NULL;
}

/**
 * Clear a hashtable, and optionally free the data
 *
 * @param hashtable The hashtable to clear
 * @param free_data Whether or not to free the data
 */
void velox_hashtable_clear(struct velox_hashtable * hashtable, bool free_data)
{
    if (free_data)
    {
        uint32_t index;

        for (index = 0; index < hashtable->size; ++index)
        {
            if (hashtable->data[index] != NULL)
            {
                free(hashtable->data[index]);
            }
        }
    }

    memset(hashtable->data, 0, hashtable->size * sizeof(void *));
}

/**
 * Delete a hashtable, and optionally free the data
 *
 * @param hashtable The hashtable to delete
 * @param free_data Whether or not to free the data
 */
void velox_hashtable_delete(struct velox_hashtable * hashtable, bool free_data)
{
    if (hashtable == NULL) return;

    velox_hashtable_clear(hashtable, free_data);

    free(hashtable);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

