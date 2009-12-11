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

#define ADD_TAG_KEY_BINDINGS(N) \
    add_key_binding(mod_mask, XK_ ## N, set_tag_ ## N); \
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT, XK_ ## N, move_focus_to_tag_ ## N); \
    add_key_binding(mod_mask | XCB_MOD_MASK_CONTROL, XK_ ## N, NULL);

static const uint32_t mod_mask = XCB_MOD_MASK_4;

struct mwm_key_binding_list * key_bindings = NULL;

void setup_key_bindings()
{
    /* Commands */
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,      XK_Return,  &spawn_terminal);
    add_key_binding(mod_mask,                           XK_r,       &spawn_dmenu);

    /* Window focus */
    add_key_binding(mod_mask,                       XK_h, &focus_next);
    add_key_binding(mod_mask,                       XK_t, &focus_previous);
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,  XK_h, &move_next);
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,  XK_t, &move_previous);

    /* Window operations */
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,      XK_c, &kill_focused_window);

    /* Layout modification */
    add_key_binding(mod_mask,                           XK_d, &decrease_master_factor);
    add_key_binding(mod_mask,                           XK_n, &increase_master_factor);
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,      XK_d, &increase_master_count);
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,      XK_n, &decrease_master_count);
    add_key_binding(mod_mask | XCB_MOD_MASK_CONTROL,    XK_d, &increase_column_count);
    add_key_binding(mod_mask | XCB_MOD_MASK_CONTROL,    XK_n, &decrease_column_count);

    /* Layout control */
    add_key_binding(mod_mask,                       XK_space,   &next_layout);
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,  XK_space,   &previous_layout);

    /* Quit */
    add_key_binding(mod_mask | XCB_MOD_MASK_SHIFT,  XK_q,   &quit);

    /* Tags */
    ADD_TAG_KEY_BINDINGS(1)
    ADD_TAG_KEY_BINDINGS(2)
    ADD_TAG_KEY_BINDINGS(3)
    ADD_TAG_KEY_BINDINGS(4)
    ADD_TAG_KEY_BINDINGS(5)
    ADD_TAG_KEY_BINDINGS(6)
    ADD_TAG_KEY_BINDINGS(7)
    ADD_TAG_KEY_BINDINGS(8)
    ADD_TAG_KEY_BINDINGS(9)
}

void cleanup_key_bindings()
{
    struct mwm_key_binding_list * next_bindings;
    while (key_bindings)
    {
        next_bindings = key_bindings->next;
        free(key_bindings);
        key_bindings = next_bindings;
    }
}

void add_key_binding(uint16_t modifiers, xcb_keysym_t keysym, void (* function)())
{
    struct mwm_key_binding_list * bindings;

    bindings = (struct mwm_key_binding_list *) malloc(sizeof(struct mwm_key_binding_list));
    bindings->binding.modifiers = modifiers;
    bindings->binding.keysym = keysym;
    bindings->binding.keycode = 0;
    bindings->binding.function = function;
    bindings->next = key_bindings;

    key_bindings = bindings;
}

