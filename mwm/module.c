/* mwm: mwm/module.c
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of mwm.
 *
 * mwm is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * mwm is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with mwm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#include "module-private.h"

struct mwm_list * modules;

void * open_module(const char const * name)
{
    char search_path[1024];
    char module_path[1024];
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
        snprintf(search_path, sizeof(search_path), "%s/.mwm/modules:/usr/lib/mwm/modules",
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

        snprintf(module_path, sizeof(module_path), "%s/mwm_%s.so", directory_path, name);

        printf("trying: %s...", module_path);

        handle = dlopen(module_path, RTLD_LAZY);

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

void load_module(const char const * name)
{
    void * module_handle;
    struct mwm_module * module;

    module_handle = open_module(name);

    assert(module_handle);

    module = (struct mwm_module *) malloc(sizeof(struct mwm_module));

    module->handle = module_handle;
    module->name = dlsym(module_handle, "name");
    module->initialize = dlsym(module_handle, "initialize");
    module->cleanup = dlsym(module_handle, "cleanup");

    printf("loaded module: %s\n", module->name);

    modules = mwm_list_insert(modules, module);
}

void initialize_modules()
{
    struct mwm_list * iterator;

    for (iterator = modules; iterator != NULL; iterator = iterator->next)
    {
        ((struct mwm_module *) iterator->data)->initialize();
    }
}

void cleanup_modules()
{
    struct mwm_module * module;

    while (modules)
    {
        module = (struct mwm_module *) modules->data;
        module->cleanup();

        dlclose(module->handle);

        free(module);

        modules = mwm_list_remove_first(modules);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

