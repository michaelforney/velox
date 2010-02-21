/* velox: libvelox/loop.c
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

#include "loop.h"

struct velox_loop * velox_loop_insert(struct velox_loop * loop, void * data)
{
    struct velox_loop * new_loop = (struct velox_loop *) malloc(sizeof(struct velox_loop));
    memset(new_loop, 0, sizeof(struct velox_loop));

    new_loop->data = data;

    if (loop == NULL)
    {
        /* Make a single element, infinite loop */
        new_loop->next = new_loop;
        new_loop->previous = new_loop;

        return new_loop;
    }
    else
    {
        /* Bind the new loop into the current loop */
        new_loop->next = loop;
        new_loop->previous = loop->previous;

        /* Bind the surrounding elements of the loop to the new element */
        new_loop->previous->next = new_loop;
        new_loop->next->previous = new_loop;

        return loop;
    }
}

struct velox_loop * velox_loop_remove(struct velox_loop * loop)
{
    struct velox_loop * new_loop;

    /* If there is only one element in the loop */
    if (velox_loop_is_singleton(loop))
    {
        free(loop);
        return NULL;
    }

    loop->previous->next = loop->next;
    loop->next->previous = loop->previous;

    new_loop = loop->next;

    free(loop);

    return new_loop;
}

struct velox_loop * velox_loop_delete(struct velox_loop * loop, bool free_data)
{
    while (loop != NULL)
    {
        if (free_data)
        {
            free(loop->data);
        }

        loop = velox_loop_remove(loop);
    }

    return NULL;
}

struct velox_loop * velox_loop_copy(struct velox_loop * loop)
{
    if (loop == NULL)
    {
        return NULL;
    }
    else
    {
        struct velox_loop * iterator;
        struct velox_loop * new_loop = NULL;

        iterator = loop;

        do
        {
            new_loop = velox_loop_insert(new_loop, iterator->data);
            iterator = iterator->next;
        } while (iterator != loop);

        return new_loop;
    }
}

void velox_loop_swap(struct velox_loop * first, struct velox_loop * second)
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

bool velox_loop_is_singleton(struct velox_loop * loop)
{
    return (loop == loop->previous) && (loop == loop->next);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

