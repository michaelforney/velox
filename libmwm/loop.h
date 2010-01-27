/* mwm: libmwm/loop.h
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of mwm.
 *
 * mwm is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * mwm is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with mwm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBMWM_LOOP_H
#define LIBMWM_LOOP_H

#include <stdbool.h>

struct mwm_loop
{
    void * data;
    struct mwm_loop * next;
    struct mwm_loop * previous;
};

struct mwm_loop * mwm_loop_insert(struct mwm_loop * loop, void * data);
struct mwm_loop * mwm_loop_remove(struct mwm_loop * loop);
struct mwm_loop * mwm_loop_delete(struct mwm_loop * loop, bool free_data);
struct mwm_loop * mwm_loop_copy(struct mwm_loop * loop);
void mwm_loop_swap(struct mwm_loop * first, struct mwm_loop * second);
bool mwm_loop_is_singleton(struct mwm_loop * loop);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

