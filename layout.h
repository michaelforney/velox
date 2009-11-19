/* mwm: layout.h
 *
 * Copyright (c) 2009 Michael Forney <michael@obberon.com>
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

#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdint.h>

#include "window.h"

struct mwm_layout
{
    const char * identifier;
    void (* arrange)(struct mwm_window_stack * stack);
};

struct mwm_layout_state
{
    uint16_t pad[32];
};

struct mwm_tile_layout_state
{
    uint16_t master_factor;
    uint16_t master_count;
    uint16_t columnt_count;
    uint16_t pad[29];
};

enum
{
    TILE,
    GRID
};

void tile_arrange(struct mwm_window_stack * stack);
void grid_arrange(struct mwm_window_stack * stack);

#endif

