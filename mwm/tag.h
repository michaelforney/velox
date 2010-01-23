/* mwm: mwm/tag.h
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

#ifndef MWM_TAG_H
#define MWM_TAG_H

#include <xcb/xcb.h>

#include "layout.h"
#include "loop.h"

struct mwm_tag
{
    uint64_t id;
    const char * name;
    struct mwm_loop * layout;
    struct mwm_loop * windows;
    struct mwm_loop * focus;
    struct mwm_layout_state state;
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
void cleanup_tags();

void set_tag_1();
void set_tag_2();
void set_tag_3();
void set_tag_4();
void set_tag_5();
void set_tag_6();
void set_tag_7();
void set_tag_8();
void set_tag_9();

void move_focus_to_tag_1();
void move_focus_to_tag_2();
void move_focus_to_tag_3();
void move_focus_to_tag_4();
void move_focus_to_tag_5();
void move_focus_to_tag_6();
void move_focus_to_tag_7();
void move_focus_to_tag_8();
void move_focus_to_tag_9();

#endif

