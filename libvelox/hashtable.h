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

#ifndef LIBVELOX_HASHTABLE
#define LIBVELOX_HASHTABLE

#include <stdint.h>
#include <stdbool.h>

struct velox_hashtable
{
    void ** data;
    uint32_t size;
    uint32_t (* hash_function)(const char * string);
};

uint32_t sdbm_hash(const char * string);

/**
 * Create a new hash table
 *
 * @param size The number of slots in the table
 * @param hash_function The hashing function to use
 * @return The newly created hash table
 */
struct velox_hashtable * velox_hashtable_create(uint32_t size, uint32_t (* hash_function)(const char * string));

/**
 * Looks up the value of a key in a hashtable
 *
 * @param hashtable The hashtable to lookup the value from
 * @param key The key to lookup
 * @return The value looked up
 */
void * velox_hashtable_lookup(struct velox_hashtable * hashtable, const char * key);

/**
 * Insert a value into a hashtable
 *
 * @param hashtable The hashtable to insert the value into
 * @param key The key to insert the value
 * @param value The value to insert into the hashtable
 */
void velox_hashtable_insert(struct velox_hashtable * hashtable, const char * key, void * value);

/**
 * Unset a key from a hashtable
 *
 * @param hashtable The hashtable to unset the key from
 * @param key The key to unset
 */
void velox_hashtable_unset(struct velox_hashtable * hashtable, const char * key);

/**
 * Check if a key already exists in a hashtable
 *
 * @param hashtable The hashtable to check for the key
 * @param key The key to check for
 * @return Whether or not the key exists in the hashtable
 */
bool velox_hashtable_exists(struct velox_hashtable * hashtable, const char * key);

/**
 * Clear a hashtable, and optionally free the data
 *
 * @param hashtable The hashtable to clear
 * @param free_data Whether or not to free the data
 */
void velox_hashtable_clear(struct velox_hashtable * hashtable, void (* free_function)());

/**
 * Delete a hashtable, and optionally free the data
 *
 * @param hashtable The hashtable to delete
 * @param free_data Whether or not to free the data
 */
void velox_hashtable_delete(struct velox_hashtable * hashtable, void (* free_function)());

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

