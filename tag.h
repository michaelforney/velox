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
    struct mwm_layout * layout;
};

enum
{
    TAG_TERM,
    TAG_WWW,
    TAG_IRC,
    TAG_IM,
    TAG_CODE,
    TAG_MAIL,
    TAG_GFX,
    TAG_MUSIC,
    TAG_MISC
};

struct mwm_tag * tags[64];

void setup_tags();

#endif

