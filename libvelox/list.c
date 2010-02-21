/* velox: libvelox/list.c
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

struct velox_list * velox_list_insert(struct velox_list * list, void * data)
{
    struct velox_list * new_list = (struct velox_list *) malloc(sizeof(struct velox_list));
    memset(new_list, 0, sizeof(struct velox_list));

    new_list->data = data;
    new_list->next = list;

    if (list != NULL)
    {
        new_list->previous = list->previous;
        list->previous = new_list;
    }

    return new_list;
}

struct velox_list * velox_list_append(struct velox_list * list, void * data)
{
    struct velox_list * iterator;

    if (list == NULL)
    {
        return velox_list_insert(list, data);
    }

    for (iterator = list; iterator->next != NULL; iterator = iterator->next);

    return velox_list_insert_after(iterator, data);
}

struct velox_list * velox_list_insert_after(struct velox_list * list, void * data)
{
    assert(list != NULL);

    list->next = velox_list_insert(list->next, data);
    list->next->previous = list;

    return list;
}

struct velox_list * velox_list_remove_first(struct velox_list * list)
{
    struct velox_list * new_list;

    assert(list != NULL);

    if (list->next != NULL)
    {
        list->next->previous = list->previous;
    }

    if (list->previous != NULL)
    {
        list->previous->next = list->next;
    }

    new_list = list->next;

    free(list);

    return new_list;
}

struct velox_list * velox_list_remove_last(struct velox_list * list)
{
    struct velox_list * iterator;

    assert(list != NULL);

    for (iterator = list; iterator->next != NULL; iterator = iterator->next);

    return velox_list_remove_first(iterator);
}

struct velox_list * velox_list_reverse(struct velox_list * list)
{
    struct velox_list * iterator;
    struct velox_list * next;

    for (iterator = list; iterator != NULL; iterator = iterator->previous)
    {
        next = iterator->next;

        iterator->next = iterator->previous;
        iterator->previous = next;

        next = iterator;
    }

    return next;
}

struct velox_list * velox_list_delete(struct velox_list * list, bool free_data)
{
    struct velox_list * iterator;
    struct velox_list * next;

    for (iterator = list, next = iterator; next != NULL; iterator = next)
    {
        next = iterator->next;

        if (free_data)
        {
            free(iterator->data);
        }
        free(iterator);
    }

    return NULL;
}

void velox_list_swap(struct velox_list * first, struct velox_list * second)
{
    void * data;

    assert(first);
    assert(second);

    if (first == second)
    {
        return;
    }

    data = first->data;

    first->data = second->data;
    second->data = data;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

