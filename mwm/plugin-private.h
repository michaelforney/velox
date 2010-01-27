// vim: fdm=syntax fo=croql sw=4 sts=4 ts=8

/* mwm: mwm/plugin-private.h
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

#ifndef MWM_PLUGIN_PRIVATE_H
#define MWM_PLUGIN_PRIVATE_H

#include <libmwm/list.h>

struct mwm_plugin
{
    void * handle;
    const char * name;
    void (* initialize)();
    void (* cleanup)();
};

extern struct mwm_list * plugins;

void load_plugin(const char * path);
void initialize_plugins();
void cleanup_plugins();

#endif

