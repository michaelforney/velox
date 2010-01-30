/* velox: modules/mpd.c
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

#include <stdio.h>
#include <mpd/client.h>

#include <velox/velox.h>
#include <velox/module.h>

const char name[] = "mpd";

struct mpd_connection * mpd_c;

void play_pause();
void next();
void previous();
void stop();

void initialize()
{
    printf(">>> mpd module\n");

    mpd_c = mpd_connection_new("localhost", 6600, 2000);

    MODULE_KEYBINDING(play_pause)
    MODULE_KEYBINDING(next)
    MODULE_KEYBINDING(previous)
    MODULE_KEYBINDING(stop)
}

void cleanup()
{
    printf("<<< mpd module\n");

    mpd_connection_free(mpd_c);
}

void play_pause()
{
    mpd_run_toggle_pause(mpd_c);
}

void next()
{
    mpd_run_next(mpd_c);
}

void previous()
{
    mpd_run_previous(mpd_c);
}

void stop()
{
    mpd_run_stop(mpd_c);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

