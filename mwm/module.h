// vim: fdm=syntax fo=croql sw=4 sts=4 ts=8

/* mwm: mwm/module.h
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

#ifndef MWM_MODULE_H
#define MWM_MODULE_H

#include <mwm/keybinding.h>

#define MODULE_KEYBINDING(function) \
    add_configured_key_binding(name, #function, &function);

#endif

