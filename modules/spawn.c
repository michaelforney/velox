// vim: fdm=syntax fo=croql sw=4 sts=4 ts=8

/* mwm: modules/spawn.c
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
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

#include <mwm/mwm.h>
#include <mwm/module.h>

const char name[] = "spawn";

void spawn_terminal();
void spawn_dmenu();

void initialize()
{
    printf(">>> spawn module\n");

    MODULE_KEYBINDING(spawn_terminal)
    MODULE_KEYBINDING(spawn_dmenu)
}

void cleanup()
{
    printf("<<< spawn module\n");
}

void spawn_terminal()
{
    printf("spawning terminal\n");
    char * const command[] = { "urxvt", NULL };
    spawn(command);
}

void spawn_dmenu()
{
    printf("spawning dmenu\n");
    char * const command[] = {
        "dmenu_run",
        "-b",
        "-fn", "-*-terminus-medium-*-*-*-12-*-*-*-*-*-*-*",
        "-nb", "#222222",
        "-nf", "#999999",
        "-sb", "#338833",
        "-sf", "#FFFFFF",
        NULL
    };
    spawn(command);
}

