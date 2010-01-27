/* mwm: mwm/layout.c
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "mwm.h"
#include "window.h"
#include "layout.h"

#include "layout-private.h"

struct mwm_hashtable * layouts;

void setup_layouts()
{
    layouts = mwm_hashtable_create(1024, &sdbm_hash);
}

void cleanup_layouts()
{
    mwm_hashtable_clear(layouts, true);
}

void add_layout(const char const * identifier, mwm_arrange_t arrange, struct mwm_layout_state * default_state)
{
    struct mwm_layout * layout;

    layout = (struct mwm_layout *) malloc(sizeof(struct mwm_layout));

    layout->identifier = strdup(identifier);
    layout->arrange = arrange;
    layout->default_state = *default_state;

    assert(!mwm_hashtable_exists(layouts, layout->identifier));
    mwm_hashtable_insert(layouts, layout->identifier, layout);
}

