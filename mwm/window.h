/* mwm: mwm/window.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of mwm.
 *
 * mwm is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * mwm is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with mwm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MWM_WINDOW_H
#define MWM_WINDOW_H

#include <stdbool.h>

#include <xcb/xcb.h>

#include <libmwm/list.h>
#include <libmwm/loop.h>

struct mwm_window
{
    xcb_window_t window_id;

    char * name;
    char * class;

    struct mwm_tag * tag;

    int16_t x, y;
    uint16_t width, height;
    uint16_t border_width;

    bool floating;
};

struct mwm_window * window_list_lookup(struct mwm_list * list, xcb_window_t window_id);
struct mwm_list * window_list_delete(struct mwm_list * list, xcb_window_t window_id);

struct mwm_window * window_loop_lookup(struct mwm_loop * loop, xcb_window_t window_id);
struct mwm_loop * window_loop_locate(struct mwm_loop * loop, xcb_window_t window_id);

bool window_has_protocol(xcb_window_t window, xcb_atom_t protocol);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

