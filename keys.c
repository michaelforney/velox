/* mwm: keys.c
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

#include <stdio.h>

#include "mwm.h"
#include "keys.h"

void spawn_terminal()
{
    printf("spawning terminal\n");
    const char * cmd[] = { "urxvt", NULL };
    spawn(cmd);
}

struct mwm_key_binding key_bindings[] = {
    /* Commands */
    { mod_mask | XCB_MOD_MASK_SHIFT, XK_Return, 0, &spawn_terminal },

    /* Window focus */
    { mod_mask, XK_H, 0, NULL },
    { mod_mask, XK_T, 0, NULL },

    /* Tag key bindings */
    { mod_mask, XK_1, 0, NULL },
    { mod_mask, XK_2, 0, NULL },
    { mod_mask, XK_3, 0, NULL },
    { mod_mask, XK_4, 0, NULL },
    { mod_mask, XK_5, 0, NULL },
    { mod_mask, XK_6, 0, NULL },
    { mod_mask, XK_7, 0, NULL },
    { mod_mask, XK_8, 0, NULL },
    { mod_mask, XK_9, 0, NULL }
};

