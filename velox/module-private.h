/* velox: velox/module-private.h
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

#ifndef VELOX_MODULE_PRIVATE_H
#define VELOX_MODULE_PRIVATE_H

#include <yaml.h>

#include "list.h"

struct velox_module
{
    void * handle;
    const char * name;
    void (* configure)(yaml_document_t * document);
    void (* setup)();
    void (* cleanup)();
};

struct velox_module_entry
{
    struct velox_module * module;
    struct velox_link DEFAULT_LINK_MEMBER;
};

extern struct velox_list modules;

void load_module(const char * const path);
void configure_module(const char * const name, yaml_document_t * document);
void setup_modules();
void cleanup_modules();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

