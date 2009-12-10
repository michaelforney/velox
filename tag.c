/* mwm: tag.c
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

void setup_tags()
{
    tags = (struct mwm_tag *) malloc(TAG_COUNT * sizeof(struct mwm_tag));

    tags[TERM].id = 1 << TERM;
    tags[TERM].name = "term";
    tags[TERM].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[TERM].layouts[0] = &layouts[TILE];
    tags[TERM].layouts[1] = &layouts[GRID];
    tags[TERM].layouts[2] = NULL;
    tags[TERM].layout_index = 0;
    tags[TERM].state = tags[TERM].layouts[tags[TERM].layout_index]->default_state;

    tags[WWW].id = 1 << WWW;
    tags[WWW].name = "www";
    tags[WWW].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[WWW].layouts[0] = &layouts[TILE];
    tags[WWW].layouts[1] = &layouts[GRID];
    tags[WWW].layouts[2] = NULL;
    tags[WWW].layout_index = 0;
    tags[WWW].state = tags[WWW].layouts[tags[WWW].layout_index]->default_state;

    tags[IRC].id = 1 << IRC;
    tags[IRC].name = "irc";
    tags[IRC].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[IRC].layouts[0] = &layouts[TILE];
    tags[IRC].layouts[1] = &layouts[GRID];
    tags[IRC].layouts[2] = NULL;
    tags[IRC].layout_index = 0;
    tags[IRC].state = tags[IRC].layouts[tags[IRC].layout_index]->default_state;

    tags[IM].id = 1 << IM;
    tags[IM].name = "im";
    tags[IM].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[IM].layouts[0] = &layouts[TILE];
    tags[IM].layouts[1] = &layouts[GRID];
    tags[IM].layouts[2] = NULL;
    tags[IM].layout_index = 0;
    tags[IM].state = tags[IM].layouts[tags[IM].layout_index]->default_state;

    tags[CODE].id = 1 << CODE;
    tags[CODE].name = "code";
    tags[CODE].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[CODE].layouts[0] = &layouts[TILE];
    tags[CODE].layouts[1] = &layouts[GRID];
    tags[CODE].layouts[2] = NULL;
    tags[CODE].layout_index = 0;
    tags[CODE].state = tags[CODE].layouts[tags[CODE].layout_index]->default_state;

    tags[MAIL].id = 1 << MAIL;
    tags[MAIL].name = "mail";
    tags[MAIL].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[MAIL].layouts[0] = &layouts[TILE];
    tags[MAIL].layouts[1] = &layouts[GRID];
    tags[MAIL].layouts[2] = NULL;
    tags[MAIL].layout_index = 0;
    tags[MAIL].state = tags[MAIL].layouts[tags[MAIL].layout_index]->default_state;

    tags[GFX].id = 1 << GFX;
    tags[GFX].name = "gfx";
    tags[GFX].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[GFX].layouts[0] = &layouts[TILE];
    tags[GFX].layouts[1] = &layouts[GRID];
    tags[GFX].layouts[2] = NULL;
    tags[GFX].layout_index = 0;
    tags[GFX].state = tags[GFX].layouts[tags[GFX].layout_index]->default_state;

    tags[MUSIC].id = 1 << MUSIC;
    tags[MUSIC].name = "music";
    tags[MUSIC].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[MUSIC].layouts[0] = &layouts[TILE];
    tags[MUSIC].layouts[1] = &layouts[GRID];
    tags[MUSIC].layouts[2] = NULL;
    tags[MUSIC].layout_index = 0;
    tags[MUSIC].state = tags[MUSIC].layouts[tags[MUSIC].layout_index]->default_state;

    tags[MISC].id = 1 << MISC;
    tags[MISC].name = "misc";
    tags[MISC].layouts = (struct mwm_layout **) malloc(3 * sizeof(struct mwm_layout *));
    tags[MISC].layouts[0] = &layouts[TILE];
    tags[MISC].layouts[1] = &layouts[GRID];
    tags[MISC].layouts[2] = NULL;
    tags[MISC].layout_index = 0;
    tags[MISC].state = tags[MISC].layouts[tags[MISC].layout_index]->default_state;
}

void cleanup_tags()
{
    free(tags[TERM].layouts);
    free(tags[WWW].layouts);
    free(tags[IRC].layouts);
    free(tags[IM].layouts);
    free(tags[CODE].layouts);
    free(tags[MAIL].layouts);
    free(tags[GFX].layouts);
    free(tags[MUSIC].layouts);
    free(tags[MISC].layouts);
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

