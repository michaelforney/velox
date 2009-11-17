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
    uint16_t values[32];
};

enum
{
    TILE,
    LAYOUT_SIZE
};

//extern const uint16_t layout_size;

struct mwm_layout * layouts[LAYOUT_SIZE];

void setup_layouts();

#endif
