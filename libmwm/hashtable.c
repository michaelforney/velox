// vim: fdm=syntax fo=croql noet sw=4 sts=4 ts=8

/* mwm: libmwm/loop.c
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

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
struct mwm_hashtable * mwm_hashtable_create(uint32_t size, uint32_t (* hash_function)(const char * string))
{
    struct mwm_hashtable * hashtable;

    hashtable = (struct mwm_hashtable *) malloc(sizeof(struct mwm_hashtable));
    hashtable->data = (void **) malloc(size * sizeof(void *));
    memset(hashtable->data, 0, size * sizeof(void *));
    hashtable->hash_function = hash_function;

    return hashtable;
}

/**
 * Looks up the value of a key in a hashtable
 *
 * @param hashtable The hashtable to lookup the value from
 * @param key The key to lookup
 * @return The value looked up
 */
void * mwm_hashtable_lookup(struct mwm_hashtable * hashtable, const char * key)
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
void mwm_hashtable_insert(struct mwm_hashtable * hashtable, const char * key, void * value)
{
    hashtable->data[hashtable->hash_function(key) % hashtable->size] = value;
}

/**
 * Unset a key from a hashtable
 *
 * @param hashtable The hashtable to unset the key from
 * @param key The key to unset
 */
void mwm_hashtable_unset(struct mwm_hashtable * hashtable, const char * key)
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
bool mwm_hashtable_exists(struct mwm_hashtable * hashtable, const char * key)
{
    return hashtable->data[hashtable->hash_function(key) % hashtable->size] != NULL;
}

/**
 * Clear a hashtable, and optionally free the data
 *
 * @param hashtable The hashtable to clear
 * @param free_data Whether or not to free the data
 */
void mwm_hashtable_clear(struct mwm_hashtable * hashtable, bool free_data)
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

