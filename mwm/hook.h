/* mwm: mwm/hook.h
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
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

#ifndef MWM_HOOK_H
#define MWM_HOOK_H

#include <mwm/window.h>

typedef void (* mwm_startup_hook_t)();
typedef void (* mwm_manage_hook_t)(struct mwm_window *);

void add_startup_hook(mwm_startup_hook_t hook);
void add_manage_hook(mwm_manage_hook_t hook);

#endif

