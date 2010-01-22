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

#include <stdlib.h>
#include <string.h>

#include "loop.h"

/**
 * Insert an element before the current position in a loop
 *
 * This is an O(1) operation
 *
 * @param loop The loop to insert the data into
 * @param data The data to insert into the loop
 * @return A new pointer to the loop
 */
struct mwm_loop * mwm_loop_insert(struct mwm_loop * loop, void * data)
{
    struct mwm_loop * new_loop = (struct mwm_loop *) malloc(sizeof(struct mwm_loop));
    memset(new_loop, 0, sizeof(struct mwm_loop));

    new_loop->data = data;

    if (loop == NULL)
    {
        /* Make a single element, infinite loop */
        new_loop->next = new_loop;
        new_loop->previous = new_loop;
    }
    else
    {
        /* Bind the new loop into the current loop */
        new_loop->next = loop;
        new_loop->previous = loop->previous;

        /* Bind the surrounding elements of the loop to the new element */
        new_loop->previous->next = new_loop;
        new_loop->next->previous = new_loop;
    }

    return new_loop;
}

/**
 * Remove the current element in a loop
 *
 * This is an O(1) operation
 *
 * @param loop The loop to remove the element from
 * @return A new pointer to the loop
 */
struct mwm_loop * mwm_loop_remove(struct mwm_loop * loop)
{
    struct mwm_loop * new_loop;

    /* If there is only one element in the loop */
    if (loop == loop->next && loop == loop->previous)
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

