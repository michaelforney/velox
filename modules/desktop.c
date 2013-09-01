/* velox: modules/desktop.c
 *
 * Copyright (c) 2013 Michael Forney <mforney@mforney.org>
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
#include <yaml.h>
#include <velox/velox.h>
#include <velox/workspace.h>
#include <velox/hook.h>
#include <velox/bound_layer.h>
#include <velox/free_layer.h>

static void handle_floating(union velox_argument argument);
static void handle_fullscreen(union velox_argument argument);

const char name[] = "desktop";

void configure(yaml_document_t * document)
{
    yaml_node_t * map, * key, * value;
    yaml_node_pair_t * pair;

    printf("Desktop: Loading configuration...");

    map = yaml_document_get_root_node(document);
    assert(map->type == YAML_MAPPING_NODE);

    for (pair = map->data.mapping.pairs.start;
        pair < map->data.mapping.pairs.top;
        ++pair)
    {
        key = yaml_document_get_node(document, pair->key);
        value = yaml_document_get_node(document, pair->value);

        assert(key->type == YAML_SCALAR_NODE);
    }

    printf("done\n");
}

bool setup()
{
    uint32_t index;
    const char * workspace_names[] =
        { "term", "www", "irc", "im", "code", "mail", "gfx", "music", "misc" };
    const char * layout_names[] = { "tile", "grid" };
    struct velox_layer * layers[2];

    printf("Desktop: Initializing...");

    /* Workspaces */
    for (index = 0; index < ARRAY_LENGTH(workspace_names); ++index)
    {
        layers[0] = bound_layer_new(layout_names, ARRAY_LENGTH(layout_names));
        layers[1] = free_layer_new();
        add_workspace(workspace_names[index], layers, ARRAY_LENGTH(layers));
    }

    printf("done\n");

    return true;
}

void cleanup()
{
    printf("Desktop: Cleaning up...");
    printf("done\n");
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

