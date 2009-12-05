/* mwm: hook.c
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
#include <string.h>
#include <dirent.h>

#include "mwm.h"
#include "hook.h"

typedef void (* startup_hook_t)();
typedef void (* manage_hook_t)(struct mwm_window *);

void set_wallpaper();

startup_hook_t startup_hooks[] = {
    &set_wallpaper
};

manage_hook_t manage_hooks[] = {
};

void run_startup_hooks()
{
    uint16_t startup_hooks_count = sizeof(startup_hooks) / sizeof(startup_hook_t);
    uint16_t startup_hook_index;

    for (startup_hook_index = 0; startup_hook_index < startup_hooks_count; startup_hook_index++)
    {
        startup_hooks[startup_hook_index]();
    }
}

void run_manage_hooks(struct mwm_window * window)
{
    uint16_t manage_hooks_count = sizeof(manage_hooks) / sizeof(manage_hook_t);
    uint16_t manage_hook_index;

    for (manage_hook_index = 0; manage_hook_index < manage_hooks_count; manage_hook_index++)
    {
        manage_hooks[manage_hook_index](window);
    }
}

/* Startup hooks */
void set_wallpaper()
{
    const char * wallpaper_path = "/home/michael/wallpaper";
    char ** wallpapers;
    uint16_t wallpaper_capacity = 64;
    uint16_t wallpaper_count = 0;
    uint16_t wallpaper_index;
    DIR * directory;
    struct dirent * entry;
    const char * command[] = {
        "feh",
        "--bg-scale",
        NULL,
        NULL
    };

    wallpapers = malloc(wallpaper_capacity);
    directory = opendir(wallpaper_path);

    printf("set_wallpaper()\n");

    readdir(directory); // .
    readdir(directory); // ..

    while ((entry = readdir(directory)) != NULL)
    {
        wallpaper_count++;

        if (wallpaper_count > wallpaper_capacity)
        {
            wallpaper_capacity *= 2;
            wallpapers = realloc(wallpapers, wallpaper_capacity);
        }

        wallpapers[wallpaper_count - 1] = malloc(strlen(entry->d_name) + strlen(wallpaper_path) + 2);
        sprintf(wallpapers[wallpaper_count - 1], "%s/%s", wallpaper_path, entry->d_name);
    }

    closedir(directory);

    if (wallpaper_count > 0)
    {
        /* Pick a random wallpaper */
        wallpaper_index = rand() % wallpaper_count;
        printf("setting wallpaper to: %s\n", wallpapers[wallpaper_index]);

        command[2] = wallpapers[wallpaper_index];

        /* Execute feh, the background setter */
        spawn(command);
    }

    free(wallpapers);
}

/* Manage hooks */
// TODO: Define some of these :)

