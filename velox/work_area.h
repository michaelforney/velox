/* velox: velox/work_area.h
 *
 * Copyright (c) 2010 Michael Forney <mforney@mforney.org>
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

#ifndef VELOX_WORK_AREA_H
#define VELOX_WORK_AREA_H

#include <velox/area.h>

typedef void (* velox_work_area_modifier_t)(
    const struct velox_area * screen_area,
    struct velox_area * work_area
);

void add_work_area_modifier(velox_work_area_modifier_t modifier);
void calculate_work_area(const struct velox_area * screen_area, struct velox_area * work_area);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

