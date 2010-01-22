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

#ifndef LIBMWM_HASHTABLE
#define LIBMWM_HASHTABLE

#include <stdint.h>
#include <stdbool.h>

struct mwm_hashtable
{
    void ** data;
    uint32_t size;
    uint32_t (* hash_function)(const char * string);
};

uint32_t sdbm_hash(const char * string);

struct mwm_hashtable * mwm_hashtable_create(uint32_t size, uint32_t (* hash_function)(const char * string));

void * mwm_hashtable_lookup(struct mwm_hashtable * hashtable, const char * key);
void mwm_hashtable_insert(struct mwm_hashtable * hashtable, const char * key, void * value);
void mwm_hashtable_unset(struct mwm_hashtable * hashtable, const char * key);
bool mwm_hashtable_exists(struct mwm_hashtable * hashtable, const char * key);
void mwm_hashtable_clear(struct mwm_hashtable * hashtable, bool free_data);

#endif

