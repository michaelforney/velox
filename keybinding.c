/* mwm: keybinding.c
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
#include <stdlib.h>

#include "keybinding.h"
#include "tag.h"
#include "mwm.h"

#define SETUP_KEY_BINDING(MODIFIERS, KEYSYM, FUNCTION) \
    key_bindings[index].modifiers = MODIFIERS; \
    key_bindings[index].keysym = KEYSYM; \
    key_bindings[index++].function = FUNCTION;

#define SETUP_TAG_KEY_BINDINGS(N) \
    SETUP_KEY_BINDING(mod_mask, XK_ ## N, set_tag_ ## N) \
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT, XK_ ## N, move_focus_to_tag_ ## N) \
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_CONTROL, XK_ ## N, NULL)

static const uint32_t mod_mask = XCB_MOD_MASK_4;

struct mwm_key_binding * key_bindings;
const uint16_t key_binding_count = 16 + (9 * 3);

void setup_key_bindings()
{
    uint16_t index = 0;

    key_bindings = (struct mwm_key_binding *) malloc(key_binding_count * sizeof(struct mwm_key_binding));

    /* Commands */
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_Return,  &spawn_terminal)
    SETUP_KEY_BINDING(mod_mask,                         XK_r,       &spawn_dmenu)

    /* Window focus */
    SETUP_KEY_BINDING(mod_mask,                         XK_h, &focus_next)
    SETUP_KEY_BINDING(mod_mask,                         XK_t, &focus_previous)
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_h, &move_next)
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_t, &move_previous)

    /* Window operations */
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_c, &kill_focused_window)

    /* Layout modification */
    SETUP_KEY_BINDING(mod_mask,                         XK_d, &decrease_master_factor)
    SETUP_KEY_BINDING(mod_mask,                         XK_n, &increase_master_factor)
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_d, &increase_master_count)
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_n, &decrease_master_count)
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_CONTROL,  XK_d, &increase_column_count)
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_CONTROL,  XK_n, &decrease_column_count)

    /* Layout control */
    SETUP_KEY_BINDING(mod_mask,                         XK_space,   &next_layout)
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_space,   &previous_layout)

    /* Quit */
    SETUP_KEY_BINDING(mod_mask | XCB_MOD_MASK_SHIFT,    XK_q,       &quit)

    /* Tags */
    SETUP_TAG_KEY_BINDINGS(1)
    SETUP_TAG_KEY_BINDINGS(2)
    SETUP_TAG_KEY_BINDINGS(3)
    SETUP_TAG_KEY_BINDINGS(4)
    SETUP_TAG_KEY_BINDINGS(5)
    SETUP_TAG_KEY_BINDINGS(6)
    SETUP_TAG_KEY_BINDINGS(7)
    SETUP_TAG_KEY_BINDINGS(8)
    SETUP_TAG_KEY_BINDINGS(9)
}

void cleanup_key_bindings()
{
    free(key_bindings);
}

