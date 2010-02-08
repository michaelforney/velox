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
    piece->x = area->x;
    piece->y = area->height * piece_index / pieces + area->y;
    piece->width = area->width;
    piece->height = area->height / pieces;
}

void velox_area_split_horizontally(
    const struct velox_area const * area,
    uint16_t pieces, uint16_t piece_index,
    struct velox_area * piece
)
{
    piece->x = area->width * piece_index / pieces + area->x;
    piece->y = area->y;
    piece->width = area->width / pieces;
    piece->height = area->height;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

