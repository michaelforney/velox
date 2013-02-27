/* velox: velox/module.c
 *
 * Copyright (c) 2010 Michael Forney <mforney@mforney.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#include "module-private.h"

VELOX_LIST(modules);

void * open_module(const char * const name)
{
    char search_path[1024];
    char module_path[1024];
    char directory_path[1024];
    char * start, * end;
    void * handle = NULL;
    bool searching = true;

    if (getenv("VELOX_PLUGIN_PATH") != NULL)
    {
        snprintf(search_path, sizeof(search_path), "%s", getenv("VELOX_PLUGIN_PATH"));
    }
    else
    {
        snprintf(search_path, sizeof(search_path), "%s/.velox/modules:/usr/lib/velox/modules",
            getenv("HOME")
        );
    }

    start = search_path;

    printf("Looking for module: %s...\n", name);
    while (searching)
    {
        end = strchr(start, ':');

        if (end == NULL)
        {
            strcpy(directory_path, start);
            searching = false;
        }
        else
        {
            strncpy(directory_path, start, end - start);
            directory_path[end - start] = '\0';
            start = end + 1;
        }

        snprintf(module_path, sizeof(module_path), "%s/%s.so", directory_path, name);

        printf("    %s...", module_path);

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

void load_module(const char * const name)
{
    void * module_handle;
    struct velox_module * module;
    struct velox_module_entry * entry;

    /* Find and open a module from the search path for velox modules */
    module_handle = open_module(name);

    assert(module_handle);

    /* Allocate a new module */
    module = (struct velox_module *) malloc(sizeof(struct velox_module));

    /* Lookup the appropriate symbols */
    module->handle = module_handle;
    module->name = dlsym(module_handle, "name");
    module->configure = dlsym(module_handle, "configure");
    module->setup = dlsym(module_handle, "setup");
    module->cleanup = dlsym(module_handle, "cleanup");

    /* Make sure these symbols exist */
    assert(module->name);
    assert(module->setup);
    assert(module->cleanup);

    printf("Loaded module: %s\n", module->name);

    entry = (struct velox_module_entry *) malloc(sizeof(struct velox_module_entry));
    entry->module = module;

    /* Add the module to the list of modules */
    list_append(&modules, entry);
}

void configure_module(const char * const name, yaml_document_t * document)
{
    struct velox_module_entry * entry;

    /* Search through the list of loaded modules and configure the first one
     * that matches. */
    list_for_each_entry(&modules, entry)
    {
        if (strcmp(name, entry->module->name) == 0)
        {
            if (entry->module->configure != NULL)
            {
                entry->module->configure(document);
                return;
            }
        }
    }
}

void setup_modules()
{
    struct velox_module_entry * entry;

    printf("\n** Initializing Modules **\n");

    /* Call the initialize function for each module */
    list_for_each_entry(&modules, entry)
    {
        entry->module->setup();
    }
}

void cleanup_modules()
{
    struct velox_module_entry * entry;
    struct velox_link * tmp;

    printf("\n** Cleaning Up Modules **\n");

    /* Call the cleanup function, then close and free each module */
    list_for_each_entry_safe(&modules, entry, tmp)
    {
        entry->module->cleanup();

        dlclose(entry->module->handle);

        free(entry->module);
        free(entry);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

