/* velox: velox/velox.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <mforney@mforney.org>
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
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/epoll.h>

#include "velox.h"
#include "window.h"
#include "workspace.h"
#include "hook.h"
#include "config_file.h"
#include "debug.h"
#include "list.h"
#include "modifier.h"
#include "resource.h"
#include "layer.h"
#include "bound_layer.h"
#include "free_layer.h"

#include "module-private.h"
#include "config_file-private.h"
#include "hook-private.h"
#include "layout-private.h"
#include "work_area-private.h"
#include "binding-private.h"

#define ARRAY_LENGTH(array) (sizeof (array) / sizeof (array)[0])

/* VELOX variables */
volatile sig_atomic_t running = true;
volatile sig_atomic_t clock_tick_update = true;
struct velox_workspace * active_workspace = NULL;
struct velox_area screen_area;
struct velox_area work_area;

uint16_t border_width = 2;

/* VELOX constants */
const char wm_name[] = "velox";

static void setup()
{
    setup_hooks();
    setup_bindings();
    setup_layouts();

    load_config();

    setup_modules();
    setup_workspaces();

    assert(workspaces.size > 0);
    active_workspace = workspace_at(0);
}

void commit_focus()
{
    focus_window(active_workspace->focus);
}

void focus(struct velox_window * window)
{
    workspace_set_focus(active_workspace, window);
    commit_focus();
}

void set_workspace(union velox_argument argument)
{
    uint8_t index = argument.uint8;
    struct velox_workspace * workspace = workspace_at(index);
    struct velox_layer * layer;
    struct velox_window * window;

    DEBUG_ENTER

    assert(index < workspaces.size);

    if (active_workspace == workspace) return;

    list_for_each_entry(&workspace->layers, layer)
    {
        layer_update(layer);

        list_for_each_entry(&layer->windows, window)
            show_window(window);
    }

    list_for_each_entry(&active_workspace->layers, layer)
    {
        list_for_each_entry(&layer->windows, window)
            hide_window(window);
    }

    active_workspace = workspace;
    run_hooks(active_workspace, VELOX_HOOK_WORKSPACE_CHANGED);

    if (active_workspace->focus)
        commit_focus();
}

void move_focus_to_workspace(union velox_argument argument)
{
    struct velox_layer * layer;
    struct velox_window * focus;
    uint8_t index = argument.uint8;

    DEBUG_ENTER

    if (active_workspace == workspace_at(index)) return;

    focus = active_workspace->focus;
    workspace_remove_window(active_workspace, focus);

    /* Look for a layer of the same type in the destination workspace to put
     * the window. */
    list_for_each_entry(&workspace_at(index)->layers, layer)
    {
        if (layer->interface == focus->layer->interface)
            goto found;
    }

    /* If none were found, default to the first layer. */
    layer = list_first(&workspace_at(index)->layers, typeof(*layer));

  found:
    focus->layer = layer;

    workspace_add_window(workspace_at(index), focus);
}

void next_workspace()
{
    struct velox_workspace * workspace_iterator;
    uint8_t index;

    DEBUG_ENTER

    vector_for_each_with_index(&workspaces, workspace_iterator, index)
    {
        if (workspace_iterator == active_workspace) break;
    }

    if (++index == workspaces.size)
    {
        index = 0;
    }

    set_workspace(uint8_argument(index));
}

void previous_workspace()
{
    struct velox_workspace * workspace_iterator;
    uint8_t index;

    DEBUG_ENTER

    vector_for_each_with_index(&workspaces, workspace_iterator, index)
    {
        if (workspace_iterator == active_workspace) break;
    }

    if (index-- == 0)
    {
        index = workspaces.size - 1;
    }

    set_workspace(uint8_argument(index));
}

void toggle_focus_type()
{
    /* XXX: Implement */
}

void next_layout()
{
    struct velox_layer * layer;

    DEBUG_ENTER

    if ((layer = workspace_find_layer(active_workspace, layer_is_bound)))
        bound_layer_next_layout(layer);
}

void previous_layout()
{
    struct velox_layer * layer;

    DEBUG_ENTER

    if ((layer = workspace_find_layer(active_workspace, layer_is_bound)))
        bound_layer_prev_layout(layer);
}

void focus_next()
{
    DEBUG_ENTER

    if (!active_workspace->focus)
        return;

    workspace_focus_next(active_workspace);
    commit_focus();
}

void focus_previous()
{
    DEBUG_ENTER

    if (!active_workspace->focus)
        return;

    workspace_focus_prev(active_workspace);
    commit_focus();
}

void move_next()
{
    DEBUG_ENTER

    if (!active_workspace->focus)
        return;

    workspace_swap_next(active_workspace);
    layer_update(active_workspace->focus->layer);
}

void move_previous()
{
    DEBUG_ENTER

    if (!active_workspace->focus)
        return;

    workspace_swap_prev(active_workspace);
    layer_update(active_workspace->focus->layer);
}

void toggle_floating()
{
    struct velox_window * window;
    struct velox_layer * layer;
    layer_predicate_t layer_pred;

    window = active_workspace->focus;

    if (layer_is_bound(window->layer))
        layer_pred = &layer_is_free;
    else if (layer_is_free(window->layer))
        layer_pred = &layer_is_bound;
    else
        return;

    if ((layer = workspace_find_layer(active_workspace, layer_pred)))
    {
        layer_remove_window(window->layer, window);
        layer_add_window(layer, window);
    }
}

void restack()
{
    struct velox_window * window;

    /* XXX: restack windows */
}

void spawn(char * const command[])
{
    DEBUG_ENTER

    if (fork() == 0)
    {
        setsid();
        execvp(command[0], command);
        exit(EXIT_SUCCESS);
    }
}

void catch_int(int signal)
{
    DEBUG_ENTER
    quit();
}

void catch_alarm(int signal)
{
    clock_tick_update = true;
}

void catch_chld(int signal)
{
    /* Clean up zombie processes */
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void run()
{
    struct epoll_event events[32];
    struct epoll_event event;
    int epoll_fd;
    int count;
    uint32_t index;
    sigset_t blocked_set, empty_set;
    struct itimerval timer;

    printf("\n** Main Event Loop **\n");

    /* Initialize signal masks */
    sigemptyset(&blocked_set);
    sigemptyset(&empty_set);

    sigaddset(&blocked_set, SIGALRM);
    sigprocmask(SIG_BLOCK, &blocked_set, NULL);

    /* Setup signal handlers */
    signal(SIGALRM, &catch_alarm);
    signal(SIGINT, &catch_int);
    signal(SIGCHLD, &catch_chld);

    epoll_fd = epoll_create1(EPOLL_CLOEXEC);

    if (epoll_fd == -1)
        die("Could not create epoll file descriptor\n");

    /* Start timer */
    timer.it_interval.tv_usec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_value.tv_sec = 1;

    setitimer(ITIMER_REAL, &timer, NULL);

    /* Main event loop */
    while (running)
    {
        count = epoll_pwait(epoll_fd, events, ARRAY_LENGTH(events), -1,
            &empty_set);

        if (count == -1)
        {
            if (errno == EINTR)
            {
                if (clock_tick_update)
                {
                    clock_tick_update = false;
                    run_hooks(NULL, VELOX_HOOK_CLOCK_TICK);
                }
            }

            continue;
        }

        for (index = 0; index < count; ++index)
        {
            ((void (*)()) events[index].data.ptr)();
        }
    }
}

void quit()
{
    running = false;
}

void cleanup()
{
    cleanup_modules();
    cleanup_bindings();
    cleanup_workspaces();
    cleanup_work_area_modifiers();
    cleanup_hooks();
    cleanup_resources();
}

void __attribute__((noreturn)) die(const char * const message, ...)
{
    va_list args;

    va_start(args, message);
    fputs("FATAL: ", stderr);
    vfprintf(stderr, message, args);
    fputc('\n', stderr);
    va_end(args);

    cleanup();

    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv)
{
    srand(time(NULL));

    printf("Velox Window Manager\n");

    setup();
    run_hooks(NULL, VELOX_HOOK_STARTUP);
    run();
    cleanup();

    return EXIT_SUCCESS;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

