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

struct velox_hashtable * velox_hashtable_create(uint32_t size, uint32_t (* hash_function)(const char * string));

void * velox_hashtable_lookup(struct velox_hashtable * hashtable, const char * key);
void velox_hashtable_insert(struct velox_hashtable * hashtable, const char * key, void * value);
void velox_hashtable_unset(struct velox_hashtable * hashtable, const char * key);
bool velox_hashtable_exists(struct velox_hashtable * hashtable, const char * key);
void velox_hashtable_clear(struct velox_hashtable * hashtable, bool free_data);
void velox_hashtable_delete(struct velox_hashtable * hashtable, bool free_data);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

