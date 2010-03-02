/* velox: velox/hashtable.c
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

#include <stdint.h>

const uint32_t hashtable_sizes[] = {
    89,
    179,
    359,
    719,
    1439,
    2879,
    5759,
    11519,
    23039,
    46079,
    92159,
    184319,
    368639,
    737279,
    1474559,
    2949119,
    5898239,
    11796479,
    23592959,
    47185919,
    94371839,
    188743679,
    377487359,
    754974719,
    1509949439
};

const uint8_t hashtable_sizes_length = sizeof(hashtable_sizes) / sizeof(hashtable_sizes[0]);

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

