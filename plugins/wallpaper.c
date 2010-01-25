// vim: fdm=syntax fo=croql sw=4 sts=4 ts=8

/* mwm: plugins/wallpaper.c
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "hook.h"

const char name[] = "wallpaper";

void set_wallpaper();

void initialize()
{
    printf(">>> wallpaper plugin\n");

    add_startup_hook(&set_wallpaper);
}

void cleanup()
{
    printf("<<< wallpaper plugin\n");
}

void set_wallpaper()
{
    char ** wallpapers;
    char wallpaper_path[1024];
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

    snprintf(wallpaper_path, sizeof(wallpaper_path), "%s/wallpaper", getenv("HOME"));

    wallpapers = malloc(wallpaper_capacity);
    directory = opendir(wallpaper_path);

    printf("set_wallpaper()\n");

    if (directory == NULL)
    {
        printf("could not open wallpaper directory: %s\n", wallpaper_path);
        return;
    }

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

