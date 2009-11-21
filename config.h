/* mwm: config.h
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

#define MOD_MASK XCB_MOD_MASK_4

static const struct mwm_key_binding key_bindings[] = {
    /* Commands */
    { MOD_MASK | XCB_MOD_MASK_SHIFT, XK_Return, 0, &spawn_terminal },

    /* Window focus */
    { MOD_MASK, XK_h, 0, &focus_next },
    { MOD_MASK, XK_t, 0, &focus_previous },
    { MOD_MASK | XCB_MOD_MASK_SHIFT, XK_h, 0, &move_next },
    { MOD_MASK | XCB_MOD_MASK_SHIFT, XK_t, 0, &move_previous },

    /* Window operations */
    { MOD_MASK | XCB_MOD_MASK_SHIFT, XK_C, 0, NULL },

    /* Tag key bindings */
    { MOD_MASK, XK_1, 0, NULL },
    { MOD_MASK, XK_2, 0, NULL },
    { MOD_MASK, XK_3, 0, NULL },
    { MOD_MASK, XK_4, 0, NULL },
    { MOD_MASK, XK_5, 0, NULL },
    { MOD_MASK, XK_6, 0, NULL },
    { MOD_MASK, XK_7, 0, NULL },
    { MOD_MASK, XK_8, 0, NULL },
    { MOD_MASK, XK_9, 0, NULL }
};

struct mwm_layout layouts[] = {
    { "Tile", &tile_arrange },
    { "Grid", &grid_arrange }
};

uint16_t border_color[] = { 0x9999, 0x9999, 0x9999 };
uint16_t border_focus_color[] = { 0x3333,  0x8888, 0x3333 };

const uint16_t border_width = 2;

