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

/**
 * Insert an element at the beginning of a list
 *
 * This is an O(1) operation
 *
 * @param list The list to insert the data into
 * @param data The data to insert into the list
 * @return A new pointer to the list
 */
struct velox_list * velox_list_insert(struct velox_list * list, void * data);

/**
 * Append an element to the end of a list
 *
 * This is a O(n) operation because the list must first be traversed
 *
 * @param list The list to append the data onto
 * @param data The data to append to the list
 * @return A new pointer to the list
 */
struct velox_list * velox_list_append(struct velox_list * list, void * data);

/**
 * Insert an element after the current element of a list
 *
 * This is an O(1) operation
 *
 * @param list The list to insert the data into. This must not be NULL
 * @param data The data to insert into the list
 * @return A new pointer to the list
 */
struct velox_list * velox_list_insert_after(struct velox_list * list, void * data);

/**
 * Removes the first element of a list
 *
 * This is an O(1) operation
 *
 * @param list The list to remove the first element from. This must not be NULL
 * @return A new pointer to the list
 */
struct velox_list * velox_list_remove_first(struct velox_list * list);

/**
 * Removes the last element of a list
 *
 * This is a O(n) operation because the list must first be traversed
 *
 * @param list The list to remove the last element from. This must not be NULL
 * @return A new pointer to the list
 */
struct velox_list * velox_list_remove_last(struct velox_list * list);

/**
 * Reverses the elements of a list
 *
 * This is an O(n) operation
 *
 * @param list The list to be reversed
 * @return A new pointer to the list
 */
struct velox_list * velox_list_reverse(struct velox_list * list);

/**
 * Deletes an entire list, and optionally frees its contents
 *
 * This is an O(n) operation
 *
 * @param list The list to be removed
 * @param free_data Whether or not to free the contents
 * @return A new pointer to the list
 */
struct velox_list * velox_list_delete(struct velox_list * list, bool free_data);

/**
 * Swap two elements of a list
 *
 * This is an O(1) operation
 *
 * @param first The first element to swap
 * @param second The second element to swap
 */
void velox_list_swap(struct velox_list * first, struct velox_list * second);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

