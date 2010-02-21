/* velox: libvelox/loop.h
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

#ifndef LIBVELOX_LOOP_H
#define LIBVELOX_LOOP_H

#include <stdbool.h>

struct velox_loop
{
    void * data;
    struct velox_loop * next;
    struct velox_loop * previous;
};

/**
 * Insert an element before the current position in a loop
 *
 * This is an O(1) operation
 *
 * @param loop The loop to insert the data into
 * @param data The data to insert into the loop
 * @return A new pointer to the loop
 */
struct velox_loop * velox_loop_insert(struct velox_loop * loop, void * data);

/**
 * Remove the current element in a loop
 *
 * This is an O(1) operation
 *
 * @param loop The loop to remove the element from
 * @return A new pointer to the loop
 */
struct velox_loop * velox_loop_remove(struct velox_loop * loop);

/**
 * Delete an entire loop, and optionally free the data
 *
 * This is an O(n) operation
 *
 * @param loop The loop to delete
 * @param free_data Whether or not to free the data enclosed in the loop
 * @return A new pointer to the loop, in this case NULL
 */
struct velox_loop * velox_loop_delete(struct velox_loop * loop, bool free_data);

/**
 * Copies an entire loop
 *
 * This is an O(n) operation
 *
 * @param loop The loop to copy
 * @return A copy of the loop
 */
struct velox_loop * velox_loop_copy(struct velox_loop * loop);

/**
 * Swap two elements of a loop
 *
 * This is an O(1) operation
 *
 * @param first The first element to swap
 * @param second The second element to swap
 */
void velox_loop_swap(struct velox_loop * first, struct velox_loop * second);

/**
 * Determines whether or not a loop contains a single element
 *
 * @param loop The loop to check
 * @return True if the loop contains a single element, false otherwise
 */
bool velox_loop_is_singleton(struct velox_loop * loop);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

