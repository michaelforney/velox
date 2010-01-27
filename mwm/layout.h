/* mwm: mwm/layout.h
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

#ifndef MWM_LAYOUT_H
#define MWM_LAYOUT_H

#include <stdint.h>

#include <libmwm/loop.h>
#include <libmwm/hashtable.h>

struct mwm_layout_state
{
    uint16_t pad[32];
};

typedef void (* mwm_arrange_t)(struct mwm_loop * list, struct mwm_layout_state * state);

struct mwm_layout
{
    const char * identifier;
    mwm_arrange_t arrange;
    struct mwm_layout_state default_state;
};

void add_layout(const char const * identifier, mwm_arrange_t arrange, struct mwm_layout_state * default_state);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

