/* velox: velox/hashtable.h
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

#ifndef VELOX_HASHTABLE
#define VELOX_HASHTABLE

#include <stdint.h>
#include <stdbool.h>

#define DEFINE_HASHTABLE(type, key_type, value_type)                        \
    struct type                                                             \
    {                                                                       \
        struct                                                              \
        {                                                                   \
            uint32_t hash;                                                  \
            value_type value;                                               \
        } * data;                                                           \
        uint32_t size;                                                      \
        uint32_t (* function)(key_type key);                                \
    }

extern const uint32_t hashtable_sizes[];
extern const uint8_t hashtable_sizes_length;

/* SDBM hashing function */
static inline uint32_t sdbm_hash(const char * string)
{
    uint32_t hash = 0;
    int32_t c;

    while ((c = *string++))
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

static inline uint32_t find_optimal_size(uint32_t size)
{
    uint8_t index;

    for (index = 0; size > hashtable_sizes[index] && index < hashtable_sizes_length; ++index);

    return hashtable_sizes[index];
}

/**
 * Create a new hash table
 *
 * @param size The number of slots in the table
 * @param hash_function The hashing function to use
 * @return The newly created hash table
 */
#define hashtable_initialize(hashtable, initial_size, hash_function)        \
{                                                                           \
    (hashtable)->size = find_optimal_size(initial_size);                    \
    (hashtable)->data = (typeof((hashtable)->data))                         \
        malloc((hashtable)->size * sizeof(typeof(*(hashtable)->data)));     \
    (hashtable)->function = hash_function;                                  \
}

/**
 * Look up the index of a key in a hashtable
 *
 * @param hashtable The hashtable to lookup from
 * @param key The key to lookup
 * @return The index of the key in the hashtable
 */
#define hashtable_lookup_index(hashtable, key)                              \
({                                                                          \
    uint32_t hash;                                                          \
    uint32_t index;                                                         \
                                                                            \
    hash = (hashtable)->function(key);                                      \
    index = hash % (hashtable)->size;                                       \
    while ((hashtable)->data[index].hash != hash) ++index;                  \
    index;                                                                  \
})

/**
 * Look up the value of a key in a hashtable
 *
 * @param hashtable The hashtable to lookup the value from
 * @param key The key to lookup
 * @return The value looked up
 */
#define hashtable_lookup(hashtable, key)                                    \
    (hashtable)->data[hashtable_lookup_index(hashtable, key)].value         \

/**
 * Insert a value into a hashtable
 *
 * @param hashtable The hashtable to insert the value into
 * @param key The key to insert the value
 * @param item The value to insert into the hashtable
 */
#define hashtable_insert(hashtable, key, item)                              \
{                                                                           \
    uint32_t hash;                                                          \
    uint32_t index;                                                         \
                                                                            \
    hash = (hashtable)->function(key);                                      \
    index = hash % (hashtable)->size;                                       \
    while ((hashtable)->data[index].hash > 0) ++index;                      \
    (hashtable)->data[index].hash = hash;                                   \
    (hashtable)->data[index].value = item;                                  \
}

/**
 * Unset a key from a hashtable
 *
 * @param hashtable The hashtable to unset the key from
 * @param key The key to unset
 */
#define hashtable_unset(hashtable, key)                                     \
    (hashtable)->data[hashtable_lookup_index(hashtable, key)].hash = 0

/**
 * Free a hashtable's data
 *
 * @param hashtable The hashtable to free
 */
#define hashtable_free(hashtable) free((hashtable)->data)

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

