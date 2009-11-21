/* mwm: tag.h
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

#ifndef TAG_H
#define TAG_H

#include "layout.h"

struct mwm_tag
{
    uint64_t id;
    const char * name;
    struct mwm_layout ** layouts;
};

enum
{
    TERM,
    WWW,
    IRC,
    IM,
    CODE,
    MAIL,
    GFX,
    MUSIC,
    MISC,
    TAG_COUNT
};

struct mwm_tag * tags;

void setup_tags();

#endif

