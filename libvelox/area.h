/* velox: libvelox/area.h
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

#ifndef LIBVELOX_AREA_H
#define LIBVELOX_AREA_H

#include <stdint.h>

struct velox_area
{
    uint32_t x0, y0, x1, y1;
};

void velox_area_split_vertically(
    const struct velox_area const * area,
    uint16_t pieces, uint16_t piece_index,
    struct velox_area * piece
);

void velox_area_split_horizontally(
    const struct velox_area const * area,
    uint16_t pieces, uint16_t piece_index,
    struct velox_area * piece
);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

