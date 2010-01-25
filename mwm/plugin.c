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
#include <dlfcn.h>
#include <assert.h>

#include "plugin.h"

struct mwm_list * plugins;

void load_plugin(const char * path)
{
    void * plugin_handle;
    struct mwm_plugin * plugin;

    plugin_handle = dlopen(path, RTLD_NOW);

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

