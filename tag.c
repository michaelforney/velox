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

#include "tag.h"

void setup_tags()
{
    tags[TAG_TERM] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_TERM]->id = 1 << TAG_TERM;
    tags[TAG_TERM]->name = "term";

    tags[TAG_WWW] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_WWW]->id = 1 << TAG_WWW;
    tags[TAG_WWW]->name = "www";

    tags[TAG_IRC] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_IRC]->id = 1 << TAG_IRC;
    tags[TAG_IRC]->name = "irc";

    tags[TAG_IM] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_IM]->id = 1 << TAG_IM;
    tags[TAG_IM]->name = "im";

    tags[TAG_CODE] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_CODE]->id = 1 << TAG_CODE;
    tags[TAG_CODE]->name = "code";

    tags[TAG_MAIL] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_MAIL]->id = 1 << TAG_MAIL;
    tags[TAG_MAIL]->name = "mail";

    tags[TAG_GFX] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_GFX]->id = 1 << TAG_GFX;
    tags[TAG_GFX]->name = "gfx";

    tags[TAG_MUSIC] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_MUSIC]->id = 1 << TAG_MUSIC;
    tags[TAG_MUSIC]->name = "music";

    tags[TAG_MISC] = (struct mwm_tag *) malloc(sizeof(struct mwm_tag));
    tags[TAG_MISC]->id = 1 << TAG_MISC;
    tags[TAG_MISC]->name = "misc";
}

void cleanup_tags()
{
    free(tags[TAG_TERM]);
    free(tags[TAG_WWW]);
    free(tags[TAG_IRC]);
    free(tags[TAG_IM]);
    free(tags[TAG_CODE]);
    free(tags[TAG_MAIL]);
    free(tags[TAG_GFX]);
    free(tags[TAG_MUSIC]);
    free(tags[TAG_MISC]);
}

