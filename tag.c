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
#include <string.h>

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
    memset(&tags[TERM].state, 0, sizeof(struct mwm_layout_state));
    ((struct mwm_tile_layout_state *) &tags[TERM].state)->master_factor = 0.5;
    ((struct mwm_tile_layout_state *) &tags[TERM].state)->master_count = 1;

    tags[WWW].id = 1 << WWW;
    tags[WWW].name = "www";
    tags[WWW].layouts = (struct mwm_layout **) malloc(0);

    tags[IRC].id = 1 << IRC;
    tags[IRC].name = "irc";
    tags[IRC].layouts = (struct mwm_layout **) malloc(0);

    tags[IM].id = 1 << IM;
    tags[IM].name = "im";
    tags[IM].layouts = (struct mwm_layout **) malloc(0);

    tags[CODE].id = 1 << CODE;
    tags[CODE].name = "code";
    tags[CODE].layouts = (struct mwm_layout **) malloc(0);

    tags[MAIL].id = 1 << MAIL;
    tags[MAIL].name = "mail";
    tags[MAIL].layouts = (struct mwm_layout **) malloc(0);

    tags[GFX].id = 1 << GFX;
    tags[GFX].name = "gfx";
    tags[GFX].layouts = (struct mwm_layout **) malloc(0);

    tags[MUSIC].id = 1 << MUSIC;
    tags[MUSIC].name = "music";
    tags[MUSIC].layouts = (struct mwm_layout **) malloc(0);

    tags[MISC].id = 1 << MISC;
    tags[MISC].name = "misc";
    tags[MISC].layouts = (struct mwm_layout **) malloc(0);
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

