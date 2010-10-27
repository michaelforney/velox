/* velox: velox/hook.h
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

#ifndef VELOX_HOOK_H
#define VELOX_HOOK_H

#include <velox/window.h>
#include <velox/list.h>

typedef void (* velox_hook_t)(void * arg);

struct velox_hook_entry
{
    velox_hook_t hook;
    struct list_head head;
};

enum velox_hook_type
{
    VELOX_HOOK_STARTUP,
    VELOX_HOOK_MANAGE_PRE,
    VELOX_HOOK_MANAGE_POST,
    VELOX_HOOK_UNMANAGE,
    VELOX_HOOK_TAG_CHANGED,
    VELOX_HOOK_ROOT_RESIZED,
    VELOX_HOOK_FOCUS_CHANGED,
    VELOX_HOOK_CLOCK_TICK,
    VELOX_HOOK_WINDOW_NAME_CHANGED
};

void add_hook(velox_hook_t hook, enum velox_hook_type type);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

