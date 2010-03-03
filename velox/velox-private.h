/* velox: velox/velox-private.h
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
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

#ifndef VELOX_VELOX_PRIVATE_H
#define VELOX_VELOX_PRIVATE_H

#include <stdint.h>

extern xcb_window_t root;

extern uint16_t pending_unmaps;

void manage(xcb_window_t window_id);
void unmanage(struct velox_window_entry * entry);
void focus(xcb_window_t window_id);
struct velox_window_entry * lookup_window_entry(xcb_window_t window_id);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

