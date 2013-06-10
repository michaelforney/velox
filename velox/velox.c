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

#include "module-private.h"
#include "config_file-private.h"
#include "hook-private.h"
#include "layout-private.h"
#include "work_area-private.h"
#include "binding-private.h"

#if WITH_X11
#   include "x11/x11.h"
#   include "x11/atom.h"
#   include "x11/x11-private.h"
#endif

#define ARRAY_LENGTH(array) (sizeof (array) / sizeof (array)[0])

/* VELOX variables */
volatile sig_atomic_t running = true;
volatile sig_atomic_t clock_tick_update = true;
struct velox_workspace * workspace = NULL;
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

#if WITH_X11
    setup_x11();
#endif

    load_config();

    setup_modules();
    setup_workspaces();

#ifdef WITH_X11
    sync_atoms();
    grab_buttons();
    run_hooks(NULL, VELOX_HOOK_KEYBOARD_MAPPING_CHANGED);
#endif

    assert(workspaces.size > 0);
    workspace = workspace_at(0);
}

void show_window(struct velox_window * window)
{
#ifdef WITH_X11
    show_x11_window(window);
#endif
}

void hide_window(struct velox_window * window)
{
#ifdef WITH_X11
    hide_x11_window(window);
#endif
}

void focus(struct velox_window * window)
{
#ifdef WITH_X11
    focus_x11_window(window);
#endif
}

void update_focus(struct velox_workspace * workspace)
{
    if (workspace->focus_type == TILE)
    {
        if (list_is_empty(&workspace->tiled.windows)) focus(NULL);
        else
        {
            focus(link_entry(workspace->tiled.focus, struct velox_window_entry)
                ->window);
        }
    }
    else
    {
        if (list_is_empty(&workspace->floated.windows)) focus(NULL);
        else
        {
            focus(list_first(&workspace->floated.windows,
                struct velox_window_entry)->window);
        }
    }
}

void set_workspace(union velox_argument argument)
{
    uint8_t index = argument.uint8;

    DEBUG_ENTER

    assert(index < workspaces.size);

    if (workspace == workspace_at(index)) return; // Nothing to do...
    else
    {
        struct velox_window_entry * window_entry;
        struct velox_window * window;

        /* Show the windows now visible */
        list_for_each_entry(&workspace_at(index)->tiled.windows, window_entry)
        {
            show_window(window_entry->window);
        }

        list_for_each_entry(&workspace_at(index)->floated.windows, window_entry)
        {
            show_window(window_entry->window);
        }

        update_focus(workspace_at(index));

        /* Hide windows no longer visible */
        list_for_each_entry(&workspace->tiled.windows, window_entry)
        {
            hide_window(window_entry->window);
        }

        list_for_each_entry(&workspace->floated.windows, window_entry)
        {
            hide_window(window_entry->window);
        }

        workspace = workspace_at(index);

        if (workspace->focus_type == TILE)
        {
            arrange();
        }

        run_hooks(workspace, VELOX_HOOK_TAG_CHANGED);

        xcb_flush(c);
    }
}

void move_focus_to_workspace(union velox_argument argument)
{
    uint8_t index = argument.uint8;

    DEBUG_ENTER

    if (workspace->focus_type == TILE)
    {
        if (list_is_empty(&workspace->tiled.windows)) return;
        else
        {
            struct velox_link * next_focus;
            struct velox_window * window;

            window = link_entry(workspace->tiled.focus, struct velox_window_entry)->window;
            next_focus = list_next(&workspace->tiled.windows, workspace->tiled.focus);

            /* Move the focus from the old list to the new list */
            link_move_after(workspace->tiled.focus,
                &workspace_at(index)->tiled.windows.head);

            if (list_is_singular(&workspace_at(index)->tiled.windows))
            {
                /* If the workspace was empty before, set its focus to the new window */
                workspace_at(index)->tiled.focus = list_first_link
                    (&workspace_at(index)->tiled.windows);
            }

            if (list_is_empty(&workspace->tiled.windows))
            {
                next_focus = &workspace->tiled.windows.head;

                if (!list_is_empty(&workspace->floated.windows))
                {
                    /* Switch focus type to float if those are the only windows
                     * on this workspace. */
                    workspace->focus_type = FLOAT;
                }
            }

            workspace->tiled.focus = next_focus;

            update_focus(workspace);
            hide_window(window);
            arrange();

            /* If the new workspace only has tiling windows, set its focus type
             * to tile */
            if (list_is_empty(&workspace_at(index)->floated.windows))
            {
                workspace_at(index)->focus_type = TILE;
            }
        }
    }
    else if (workspace->focus_type == FLOAT)
    {
        if (list_is_empty(&workspace->floated.windows)) return;
        else
        {
            struct velox_window_entry * entry;
            struct velox_window * window;

            entry = list_first(&workspace->floated.windows,
                struct velox_window_entry);
            window = entry->window;

            list_del(entry);
            list_append(&workspace_at(index)->floated.windows, entry);

            /* Switch focus type to tile if those are the only windows on this
             * workspace */
            if (list_is_empty(&workspace->floated.windows))
            {
                workspace->focus_type = TILE;
            }

            update_focus(workspace);
            hide_window(window);
            arrange();

            if (list_is_empty(&workspace_at(index)->tiled.windows))
            {
                workspace_at(index)->focus_type = FLOAT;
            }
        }
    }

    xcb_flush(c);
}

void set_focus_type(enum velox_workspace_focus_type focus_type)
{
    if (workspace->focus_type == focus_type) return;

    if (focus_type == TILE && !list_is_empty(&workspace->tiled.windows))
    {
        workspace->focus_type = focus_type;

        focus(link_entry(workspace->tiled.focus, struct velox_window_entry)
            ->window);
    }
    else if (focus_type == FLOAT && !list_is_empty(&workspace->floated.windows))
    {
        workspace->focus_type = focus_type;

        focus(list_first(&workspace->floated.windows, struct velox_window_entry)
            ->window);
    }
}

void next_workspace()
{
    struct velox_workspace * workspace_iterator;
    uint8_t index;

    DEBUG_ENTER

    vector_for_each_with_index(&workspaces, workspace_iterator, index)
    {
        if (workspace_iterator == workspace) break;
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
        if (workspace_iterator == workspace) break;
    }

    if (index-- == 0)
    {
        index = workspaces.size - 1;
    }

    set_workspace(uint8_argument(index));
}

void toggle_focus_type()
{
    if (workspace->focus_type == TILE)  set_focus_type(FLOAT);
    else                                set_focus_type(TILE);
}

void set_layout(struct velox_link * link)
{
    struct velox_layout * layout;

    workspace->layout = link;
    layout = link_entry(link, struct velox_layout_entry)->layout;
    memcpy(&workspace->state, layout->default_state,
        layout->default_state_size);

    arrange();
}

void next_layout()
{
    DEBUG_ENTER

    set_layout(list_next(&workspace->layouts, workspace->layout));
}

void previous_layout()
{
    DEBUG_ENTER

    set_layout(list_prev(&workspace->layouts, workspace->layout));
}

void focus_next()
{
    DEBUG_ENTER

    if (workspace->focus_type == TILE)
    {
        if (list_is_trivial(&workspace->tiled.windows))
        {
            return;
        }

        workspace->tiled.focus = list_next(&workspace->tiled.windows,
            workspace->tiled.focus);

        focus(link_entry(workspace->tiled.focus, struct velox_window_entry)
            ->window);
    }
    else if (workspace->focus_type == FLOAT)
    {
        struct velox_window_entry * entry;

        if (list_is_trivial(&workspace->floated.windows))
        {
            return;
        }

        entry = list_last(&workspace->floated.windows,
            struct velox_window_entry);

        link_move_after(&entry->link, &workspace->floated.windows.head);

        focus(entry->window);
        restack();
    }
}

void focus_previous()
{
    DEBUG_ENTER

    if (workspace->focus_type == TILE)
    {
        if (list_is_trivial(&workspace->tiled.windows))
        {
            return;
        }

        workspace->tiled.focus = list_prev(&workspace->tiled.windows,
            workspace->tiled.focus);

        focus(link_entry(workspace->tiled.focus, struct velox_window_entry)
            ->window);
    }
    else if (workspace->focus_type == FLOAT)
    {
        struct velox_window_entry * entry;

        if (list_is_trivial(&workspace->floated.windows))
        {
            return;
        }

        entry = list_first(&workspace->floated.windows,
            struct velox_window_entry);

        link_move_before(&entry->link, &workspace->floated.windows.head);

        focus(entry->window);
        restack();
    }
}

void move_next()
{
    DEBUG_ENTER

    if (workspace->focus_type == TILE)
    {
        struct velox_window_entry * first, * second;
        struct velox_window * first_window;

        if (list_is_trivial(&workspace->tiled.windows)) return;

        first = link_entry(workspace->tiled.focus, struct velox_window_entry);
        second = link_entry(list_next(&workspace->tiled.windows,
            workspace->tiled.focus), struct velox_window_entry);

        /* Swap the two windows */
        first_window = first->window;
        first->window = second->window;
        second->window = first_window;

        workspace->tiled.focus = list_next(&workspace->tiled.windows,
            workspace->tiled.focus);

        arrange();
    }
}

void move_previous()
{
    DEBUG_ENTER

    if (workspace->focus_type == TILE)
    {
        struct velox_window_entry * first, * second;
        struct velox_window * first_window;

        if (list_is_trivial(&workspace->tiled.windows)) return;

        first = link_entry(workspace->tiled.focus, struct velox_window_entry);
        second = link_entry(list_prev(&workspace->tiled.windows,
            workspace->tiled.focus), struct velox_window_entry);

        /* Swap the two windows */
        first_window = first->window;
        first->window = second->window;
        second->window = first_window;

        workspace->tiled.focus = list_prev(&workspace->tiled.windows,
            workspace->tiled.focus);

        arrange();
    }
}

void toggle_floating()
{
    struct velox_window_entry * entry;

    if (workspace->focus_type == TILE)
    {
        if (!list_is_empty(&workspace->tiled.windows))
        {
            entry = link_entry(workspace->tiled.focus, struct velox_window_entry);
            workspace->tiled.focus = list_next(&workspace->tiled.windows,
                workspace->tiled.focus);

            link_move_after(&entry->link, &workspace->floated.windows.head);

            entry->window->floating = true;
            workspace->focus_type = FLOAT;

            update_focus(workspace);
            restack();
            arrange();
        }
    }
    else
    {
        if (!list_is_empty(&workspace->floated.windows))
        {
            entry = list_first(&workspace->floated.windows,
                struct velox_window_entry);

            link_move_after(&entry->link, &workspace->tiled.windows.head);

            workspace->tiled.focus = &entry->link;

            entry->window->floating = false;
            workspace->focus_type = TILE;

            update_focus(workspace);
            restack();
            arrange();
        }
    }
}

void arrange()
{
    DEBUG_ENTER

    if (list_is_empty(&workspace->tiled.windows)) return;

    assert(!list_is_empty(&workspace->layouts));

    calculate_work_area(&screen_area, &work_area);
    link_entry(workspace->layout, struct velox_layout_entry)->layout->arrange
        (&work_area, &workspace->tiled.windows, &workspace->state);

    clear_event_type = XCB_ENTER_NOTIFY;
}

void restack()
{
    uint32_t mask = XCB_CONFIG_WINDOW_STACK_MODE;
    uint32_t values[] = { XCB_STACK_MODE_ABOVE };
    struct velox_window_entry * entry;

    /* Stack the floating windows */
    list_for_each_entry_reverse(&workspace->floated.windows, entry)
    {
        xcb_configure_window(c, entry->window->window_id, mask, values);
    }

    clear_event_type = XCB_ENTER_NOTIFY;
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

#if WITH_X11
    event.events = EPOLLIN;
    event.data.ptr = &handle_x11_data;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, x11_fd, &event);
#endif

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

#ifdef WITH_X11
    cleanup_x11();
#endif
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
    manage_existing_windows();
    run_hooks(NULL, VELOX_HOOK_STARTUP);
    run();
    cleanup();

    return EXIT_SUCCESS;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

