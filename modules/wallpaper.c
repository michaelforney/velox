/* velox: modules/wallpaper.c
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of velox.
 *
 * velox is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * velox is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with velox.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <yaml.h>

#include <velox/velox.h>
#include <velox/hook.h>
#include <velox/debug.h>

const char name[] = "wallpaper";

static const char * path = "wallpaper";

static void set_wallpaper();

void configure(yaml_document_t * document)
{
    yaml_node_t * map;
    yaml_node_pair_t * pair;

    yaml_node_t * key, * value;

    printf("Wallpaper: Loading configuration...");

    map = yaml_document_get_root_node(document);
    assert(map->type == YAML_MAPPING_NODE);

    for (pair = map->data.mapping.pairs.start;
        pair < map->data.mapping.pairs.top;
        ++pair)
    {
        key = yaml_document_get_node(document, pair->key);
        value = yaml_document_get_node(document, pair->value);

        assert(key->type == YAML_SCALAR_NODE);

        if (strcmp((const char const *) key->data.scalar.value, "path") == 0)
        {
            assert(value->type == YAML_SCALAR_NODE);

            /* TODO: Free this when cleaning up. I'm not sure how to
             * differentiate between a configured value and the default, so for
             * now, we will accept a memory leak. */
            path = strdup((const char const *) value->data.scalar.value);
        }
    }

    printf("done\n    Path: %s\n", path);
}

void initialize(void * arg)
{
    printf("Wallpaper: Initializing...");

    add_hook(&set_wallpaper, VELOX_HOOK_STARTUP);

    printf("done\n");
}

void cleanup()
{
    printf("Wallpaper: Cleaning up...");
    printf("done\n");
}

static void set_wallpaper(void * arg)
{
    char * wallpaper;
    uint16_t wallpaper_count = 0;
    uint16_t wallpaper_index;
    DIR * directory;
    struct dirent * entry;
    char * command[] = {
        "feh",
        "--bg-scale",
        NULL,
        NULL
    };

    directory = opendir(path);

    DEBUG_ENTER

    if (directory == NULL)
    {
        DEBUG_PRINT("could not open wallpaper directory: %s\n", path)
        return;
    }

    readdir(directory); // .
    readdir(directory); // ..

    while ((entry = readdir(directory)) != NULL)
    {
        wallpaper_count++;

    }

    if (wallpaper_count > 0)
    {
        uint16_t index;

        rewinddir(directory);

        readdir(directory); // .
        readdir(directory); // ..

        wallpaper_index = rand() % wallpaper_count;

        for (index = 0; index < wallpaper_index; ++index, readdir(directory));

        entry = readdir(directory);

        {
            char wallpaper[strlen(entry->d_name) + strlen(path) + 2];

            sprintf(wallpaper, "%s/%s", path, entry->d_name);
            closedir(directory);

            DEBUG_PRINT("setting wallpaper to: %s\n", wallpaper)
            command[2] = wallpaper;

            /* Execute feh, the background setter */
            spawn(command);
        }
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

