/* velox: velox/atom.c
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

#include "velox.h"
#include "atom.h"
#include "vector.h"

xcb_atom_t WM_PROTOCOLS, WM_DELETE_WINDOW, WM_STATE;

struct atom_request
{
    xcb_intern_atom_cookie_t cookie;
    xcb_atom_t * atom;
};

DEFINE_VECTOR(atom_request_list, struct atom_request);

struct atom_request_list atom_requests;

static void __attribute__((constructor)) initialize_client_list()
{
    vector_initialize(&atom_requests, 64);
}

static void __attribute__((destructor)) free_client_list()
{
    vector_free(&atom_requests);
}

void register_atom(const char * const name, xcb_atom_t * atom)
{
    struct atom_request * request = vector_append_address(&atom_requests);

    request->cookie = xcb_intern_atom(c, false, strlen(name), name);
    request->atom = atom;
}

void sync_atoms()
{
    struct atom_request * request;
    xcb_intern_atom_reply_t * reply;

    vector_for_each(&atom_requests, request)
    {
        reply = xcb_intern_atom_reply(c, request->cookie, NULL);
        *request->atom = reply->atom;
    }

    vector_clear(&atom_requests);
}

void setup_atoms()
{
#define ATOM(name) register_atom(#name, &name);
    ATOM(WM_PROTOCOLS)
    ATOM(WM_DELETE_WINDOW)
    ATOM(WM_STATE)
#undef ATOM
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

