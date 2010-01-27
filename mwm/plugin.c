// vim: fdm=syntax fo=croql sw=4 sts=4 ts=8

/* mwm: mwm/plugin.c
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#include "plugin-private.h"

struct mwm_list * plugins;

void * open_plugin(const char const * name)
{
    char search_path[1024];
    char plugin_path[1024];
    char * start, * end;
    char * directory_path;
    void * handle = NULL;
    bool searching = true;

    if (getenv("MWM_PLUGIN_PATH") != NULL)
    {
        snprintf(search_path, sizeof(search_path), "%s", getenv("MWM_PLUGIN_PATH"));
    }
    else
    {
        snprintf(search_path, sizeof(search_path), "%s/.mwm/plugins:/usr/lib/mwm/plugins",
            getenv("HOME")
        );
    }

    start = search_path;

    while (searching)
    {
        end = strchr(start, ':');

        if (end == NULL)
        {
            directory_path = strdup(start);
            searching = false;
        }
        else
        {
            directory_path = strndup(start, end - start);
            start = end + 1;
        }

        snprintf(plugin_path, sizeof(plugin_path), "%s/mwm_%s.so", directory_path, name);

        printf("trying: %s...", plugin_path);

        handle = dlopen(plugin_path, RTLD_LAZY);

        if (handle)
        {
            printf("found\n");
            searching = false;
        }
        else
        {
            printf("not found\n");
        }
    }

    return handle;
}

void load_plugin(const char const * name)
{
    void * plugin_handle;
    struct mwm_plugin * plugin;

    plugin_handle = open_plugin(name);

    assert(plugin_handle);

    plugin = (struct mwm_plugin *) malloc(sizeof(struct mwm_plugin));

    plugin->handle = plugin_handle;
    plugin->name = dlsym(plugin_handle, "name");
    plugin->initialize = dlsym(plugin_handle, "initialize");
    plugin->cleanup = dlsym(plugin_handle, "cleanup");

    printf("loaded plugin: %s\n", plugin->name);

    plugins = mwm_list_insert(plugins, plugin);
}

void initialize_plugins()
{
    struct mwm_list * iterator;

    for (iterator = plugins; iterator != NULL; iterator = iterator->next)
    {
        ((struct mwm_plugin *) iterator->data)->initialize();
    }
}

void cleanup_plugins()
{
    struct mwm_plugin * plugin;

    while (plugins)
    {
        plugin = (struct mwm_plugin *) plugins->data;
        plugin->cleanup();

        dlclose(plugin->handle);

        free(plugin);

        plugins = mwm_list_remove_first(plugins);
    }
}

