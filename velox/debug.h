/* velox: velox/debug.h
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

#ifndef VELOX_DEBUG_H
#define VELOX_DEBUG_H

#ifdef VELOX_DEBUG
#   include <stdio.h>
#   define DEBUG_ENTER \
        fprintf(stderr, "# %s\n", __func__);
#   define DEBUG_PRINT(fmt, args...)\
        fprintf(stderr, "# [%u] " fmt, __LINE__, ## args);
#else
#   define DEBUG_ENTER
#   define DEBUG_RETURN
#   define DEBUG_PRINT(fmt, args...)
#endif

#endif

