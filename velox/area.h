/* velox: velox/area.h
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

#ifndef VELOX_AREA_H
#define VELOX_AREA_H

#include <stdint.h>

struct velox_area
{
    uint32_t x, y;
    uint32_t width, height;
};

/**
 * Split an array vertically into the specified number of pieces, and set the
 * destination piece to the piece at the specified index
 *
 * @param area The area to split
 * @param pieces The number of pieces in which to split the area
 * @param piece_index The index of the destination piece
 * @param piece The destination piece
 */
static inline void velox_area_split_vertically(
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

/**
 * Split an array horizontally into the specified number of pieces, and set the
 * destination piece to the piece at the specified index
 *
 * @param area The area to split
 * @param pieces The number of pieces in which to split the area
 * @param piece_index The index of the destination piece
 * @param piece The destination piece
 */
static inline void velox_area_split_horizontally(
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

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

