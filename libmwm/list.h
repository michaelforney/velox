/* mwm: libmwm/list.h
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

#ifndef LIBMWM_LIST_H
#define LIBMWM_LIST_H

#include <stdbool.h>

struct mwm_list
{
    void * data;
    struct mwm_list * next;
    struct mwm_list * previous;
};

struct mwm_list * mwm_list_insert(struct mwm_list * list, void * data);
struct mwm_list * mwm_list_append(struct mwm_list * list, void * data);

struct mwm_list * mwm_list_insert_after(struct mwm_list * list, void * data);

struct mwm_list * mwm_list_remove_first(struct mwm_list * list);
struct mwm_list * mwm_list_remove_last(struct mwm_list * list);

struct mwm_list * mwm_list_reverse(struct mwm_list * list);
struct mwm_list * mwm_list_delete(struct mwm_list * list, bool free_data);

#endif

