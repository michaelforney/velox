/* velox: libvelox/list.h
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

#ifndef LIBVELOX_LIST_H
#define LIBVELOX_LIST_H

#include <stdbool.h>

struct velox_list
{
    void * data;
    struct velox_list * next;
    struct velox_list * previous;
};

struct velox_list * velox_list_insert(struct velox_list * list, void * data);
struct velox_list * velox_list_append(struct velox_list * list, void * data);

struct velox_list * velox_list_insert_after(struct velox_list * list, void * data);

struct velox_list * velox_list_remove_first(struct velox_list * list);
struct velox_list * velox_list_remove_last(struct velox_list * list);

struct velox_list * velox_list_reverse(struct velox_list * list);
struct velox_list * velox_list_delete(struct velox_list * list, bool free_data);
void velox_list_swap(struct velox_list * first, struct velox_list * second);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

