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

struct velox_loop * velox_loop_insert(struct velox_loop * loop, void * data);
struct velox_loop * velox_loop_remove(struct velox_loop * loop);
struct velox_loop * velox_loop_delete(struct velox_loop * loop, bool free_data);
struct velox_loop * velox_loop_copy(struct velox_loop * loop);
void velox_loop_swap(struct velox_loop * first, struct velox_loop * second);
bool velox_loop_is_singleton(struct velox_loop * loop);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

