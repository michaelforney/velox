/* mwm: window.h
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

#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>

#include <xcb/xcb.h>

struct mwm_window
{
    xcb_window_t window_id;

    bool floating;
    uint16_t x, y;
    uint16_t width, height;
    uint64_t tags;
};

// typedef struct mwm_window mwm_window_t;

void window_initialize();
struct mwm_window * window_lookup(xcb_window_t window_id);
void window_insert(struct mwm_window * window);
void window_delete(xcb_window_t window_id);

#endif

