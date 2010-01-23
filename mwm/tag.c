/* mwm: mwm/tag.c
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

#include <stdlib.h>
#include <stdio.h>

#include "tag.h"
#include "mwm.h"

void setup_tags()
{
    tags = (struct mwm_tag *) malloc(TAG_COUNT * sizeof(struct mwm_tag));
    memset(tags, 0, TAG_COUNT * sizeof(struct mwm_tag));

    tags[TERM].id = 1 << TERM;
    tags[TERM].name = "term";
    tags[TERM].layout = mwm_loop_insert(tags[TERM].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[TERM].layout = mwm_loop_insert(tags[TERM].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[TERM].state = ((struct mwm_layout *) tags[TERM].layout->data)->default_state;

    tags[WWW].id = 1 << WWW;
    tags[WWW].name = "www";
    tags[WWW].layout = mwm_loop_insert(tags[WWW].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[WWW].layout = mwm_loop_insert(tags[WWW].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[WWW].state = ((struct mwm_layout *) tags[WWW].layout->data)->default_state;

    tags[IRC].id = 1 << IRC;
    tags[IRC].name = "irc";
    tags[IRC].layout = mwm_loop_insert(tags[IRC].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[IRC].layout = mwm_loop_insert(tags[IRC].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[IRC].state = ((struct mwm_layout *) tags[IRC].layout->data)->default_state;

    tags[IM].id = 1 << IM;
    tags[IM].name = "im";
    tags[IM].layout = mwm_loop_insert(tags[IM].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[IM].layout = mwm_loop_insert(tags[IM].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[IM].state = ((struct mwm_layout *) tags[IM].layout->data)->default_state;

    tags[CODE].id = 1 << CODE;
    tags[CODE].name = "code";
    tags[CODE].layout = mwm_loop_insert(tags[CODE].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[CODE].layout = mwm_loop_insert(tags[CODE].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[CODE].state = ((struct mwm_layout *) tags[CODE].layout->data)->default_state;

    tags[MAIL].id = 1 << MAIL;
    tags[MAIL].name = "mail";
    tags[MAIL].layout = mwm_loop_insert(tags[MAIL].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[MAIL].layout = mwm_loop_insert(tags[MAIL].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[MAIL].state = ((struct mwm_layout *) tags[MAIL].layout->data)->default_state;

    tags[GFX].id = 1 << GFX;
    tags[GFX].name = "gfx";
    tags[GFX].layout = mwm_loop_insert(tags[GFX].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[GFX].layout = mwm_loop_insert(tags[GFX].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[GFX].state = ((struct mwm_layout *) tags[GFX].layout->data)->default_state;

    tags[MUSIC].id = 1 << MUSIC;
    tags[MUSIC].name = "music";
    tags[MUSIC].layout = mwm_loop_insert(tags[MUSIC].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[MUSIC].layout = mwm_loop_insert(tags[MUSIC].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[MUSIC].state = ((struct mwm_layout *) tags[MUSIC].layout->data)->default_state;

    tags[MISC].id = 1 << MISC;
    tags[MISC].name = "misc";
    tags[MISC].layout = mwm_loop_insert(tags[MISC].layout, mwm_hashtable_lookup(layouts, "tile"));
    tags[MISC].layout = mwm_loop_insert(tags[MISC].layout, mwm_hashtable_lookup(layouts, "grid"));
    tags[MISC].state = ((struct mwm_layout *) tags[MISC].layout->data)->default_state;
}

void cleanup_tags()
{
    mwm_loop_delete(tags[TERM].layout, false);
    mwm_loop_delete(tags[TERM].layout, false);
    mwm_loop_delete(tags[WWW].layout, false);
    mwm_loop_delete(tags[IRC].layout, false);
    mwm_loop_delete(tags[IM].layout, false);
    mwm_loop_delete(tags[CODE].layout, false);
    mwm_loop_delete(tags[MAIL].layout, false);
    mwm_loop_delete(tags[GFX].layout, false);
    mwm_loop_delete(tags[MUSIC].layout, false);
    mwm_loop_delete(tags[MISC].layout, false);
    free(tags);
}

void set_tag_1()
{
    printf("set_tag_1\n");
    set_tag(&tags[TERM]);
}

void set_tag_2()
{
    printf("set_tag_2\n");
    set_tag(&tags[WWW]);
}

void set_tag_3()
{
    printf("set_tag_3\n");
    set_tag(&tags[IRC]);
}

void set_tag_4()
{
    printf("set_tag_4\n");
    set_tag(&tags[IM]);
}

void set_tag_5()
{
    printf("set_tag_5\n");
    set_tag(&tags[CODE]);
}

void set_tag_6()
{
    printf("set_tag_6\n");
    set_tag(&tags[MAIL]);
}

void set_tag_7()
{
    printf("set_tag_7\n");
    set_tag(&tags[GFX]);
}

void set_tag_8()
{
    printf("set_tag_8\n");
    set_tag(&tags[MUSIC]);
}

void set_tag_9()
{
    printf("set_tag_9\n");
    set_tag(&tags[MISC]);
}

void move_focus_to_tag_1()
{
    move_focus_to_tag(&tags[TERM]);
}

void move_focus_to_tag_2()
{
    move_focus_to_tag(&tags[WWW]);
}

void move_focus_to_tag_3()
{
    move_focus_to_tag(&tags[IRC]);
}

void move_focus_to_tag_4()
{
    move_focus_to_tag(&tags[IM]);
}

void move_focus_to_tag_5()
{
    move_focus_to_tag(&tags[CODE]);
}

void move_focus_to_tag_6()
{
    move_focus_to_tag(&tags[MAIL]);
}

void move_focus_to_tag_7()
{
    move_focus_to_tag(&tags[GFX]);
}

void move_focus_to_tag_8()
{
    move_focus_to_tag(&tags[MUSIC]);
}

void move_focus_to_tag_9()
{
    move_focus_to_tag(&tags[MISC]);
}

