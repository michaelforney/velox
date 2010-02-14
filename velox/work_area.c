/* velox: velox/work_area.c
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
#include <stdio.h>
#include <string.h>

#include <libvelox/list.h>

#include "velox.h"
#include "work_area.h"
#include "debug.h"

struct velox_list * work_area_modifiers = NULL;

void add_work_area_modifier(velox_work_area_modifier_t modifier)
{
    work_area_modifiers = velox_list_insert(work_area_modifiers, modifier);
}

void calculate_work_area(const struct velox_area * screen_area, struct velox_area * work_area)
{
    struct velox_list * iterator;
    struct velox_area modified_area;

    DEBUG_ENTER

    *work_area = *screen_area;

    for (iterator = work_area_modifiers; iterator != NULL; iterator = iterator->next)
    {
        ((velox_work_area_modifier_t) iterator->data)(screen_area, &modified_area);

        work_area->x = MAX(work_area->x, modified_area.x);
        work_area->y = MAX(work_area->y, modified_area.y);
        work_area->width = MIN(work_area->width, modified_area.width);
        work_area->height = MIN(work_area->height, modified_area.height);
    }

    DEBUG_PRINT("x: %u, y: %u, width: %u, height: %u\n", work_area->x, work_area->y, work_area->width, work_area->height)
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

