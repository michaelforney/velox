/* mwm: hook.h
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

#ifndef HOOK_H
#define HOOK_H

#include "window.h"

struct mwm_startup_hook
{
    uint64_t id;
    void (* hook)();
};

struct mwm_manage_hook
{
    uint64_t id;
    void (* hook)(struct mwm_window * window);
};

void run_startup_hooks();
void run_manage_hooks(struct mwm_window * window);

#endif

