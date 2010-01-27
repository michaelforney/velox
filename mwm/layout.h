/* mwm: mwm/layout.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
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

#ifndef MWM_LAYOUT_H
#define MWM_LAYOUT_H

#include <stdint.h>

#include <libmwm/loop.h>
#include <libmwm/hashtable.h>

struct mwm_layout_state
{
    uint16_t pad[32];
};

struct mwm_tile_layout_state
{
    float master_factor;
    uint16_t master_count;
    uint16_t column_count;
    uint16_t pad[28];
};

struct mwm_layout
{
    const char * identifier;
    void (* arrange)(struct mwm_loop * list, struct mwm_layout_state * state);
    struct mwm_layout_state default_state;
};

extern struct mwm_hashtable * layouts;

void setup_layouts();
void cleanup_layouts();

#endif

