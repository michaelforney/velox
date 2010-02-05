/* velox: libvelox/area.c
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

#include "area.h"

void velox_area_split_vertically(
    const struct velox_area const * area,
    uint16_t pieces, uint16_t piece_index,
    struct velox_area * piece
)
{
    piece->x0 = area->x0;
    piece->x1 = area->x1;
    piece->y0 = (area->y1 - area->y0) * piece_index / pieces + area->y0;
    piece->y1 = (area->y1 - area->y0) * (piece_index + 1) / pieces + area->y0;
}

void velox_area_split_horizontally(
    const struct velox_area const * area,
    uint16_t pieces, uint16_t piece_index,
    struct velox_area * piece
)
{
    piece->y0 = area->y0;
    piece->y1 = area->y1;
    piece->x0 = (area->x1 - area->x0) * piece_index / pieces + area->x0;
    piece->x1 = (area->x1 - area->x0) * (piece_index + 1) / pieces + area->x0;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

