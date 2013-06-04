/* velox: velox/x11/event_types.h
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

DO(XCB_KEY_PRESS,           key_press)
DO(XCB_KEY_RELEASE,         key_release)
DO(XCB_BUTTON_PRESS,        button_press)
DO(XCB_BUTTON_RELEASE,      button_release)
DO(XCB_MOTION_NOTIFY,       motion_notify)
DO(XCB_ENTER_NOTIFY,        enter_notify)
DO(XCB_LEAVE_NOTIFY,        leave_notify)
DO(XCB_FOCUS_IN,            focus_in)
DO(XCB_FOCUS_OUT,           focus_out)
DO(XCB_KEYMAP_NOTIFY,       keymap_notify)
DO(XCB_EXPOSE,              expose)
DO(XCB_GRAPHICS_EXPOSURE,   graphics_exposure)
DO(XCB_NO_EXPOSURE,         no_exposure)
DO(XCB_VISIBILITY_NOTIFY,   visibility_notify)
DO(XCB_CREATE_NOTIFY,       create_notify)
DO(XCB_DESTROY_NOTIFY,      destroy_notify)
DO(XCB_UNMAP_NOTIFY,        unmap_notify)
DO(XCB_MAP_NOTIFY,          map_notify)
DO(XCB_MAP_REQUEST,         map_request)
DO(XCB_REPARENT_NOTIFY,     reparent_notify)
DO(XCB_CONFIGURE_NOTIFY,    configure_notify)
DO(XCB_CONFIGURE_REQUEST,   configure_request)
DO(XCB_GRAVITY_NOTIFY,      gravity_notify)
DO(XCB_RESIZE_REQUEST,      resize_request)
DO(XCB_CIRCULATE_NOTIFY,    circulate_notify)
DO(XCB_CIRCULATE_REQUEST,   circulate_request)
DO(XCB_PROPERTY_NOTIFY,     property_notify)
DO(XCB_SELECTION_CLEAR,     selection_clear)
DO(XCB_SELECTION_REQUEST,   selection_request)
DO(XCB_SELECTION_NOTIFY,    selection_notify)
DO(XCB_COLORMAP_NOTIFY,     colormap_notify)
DO(XCB_CLIENT_MESSAGE,      client_message)
DO(XCB_MAPPING_NOTIFY,      mapping_notify)

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

