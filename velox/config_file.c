/* velox: velox/config_file.c
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

#include <yaml.h>
#include <assert.h>

#include "config_file.h"

#include "velox.h"
#include "module-private.h"

FILE * open_config_file(const char * name)
{
    FILE * file;
    char path[1024];

    /* First check in "${HOME}/.velox/" */
    snprintf(path, sizeof(path), "%s/.velox/%s", getenv("HOME"), name);
    if ((file = fopen(path, "r")) != NULL) return file;

    /* Then try the global "/etc/velox" */
    // TODO: Some users might not use this as the global configuration directory
    snprintf(path, sizeof(path), "/etc/velox/%s", name);
    if ((file = fopen(path, "r")) != NULL) return file;

    return NULL;
}

void parse_config()
{
    FILE * file;

    yaml_parser_t parser;
    yaml_document_t document;

    yaml_node_t * root;

    yaml_node_t * map;
    yaml_node_pair_t * pair;

    yaml_node_t * key, * value;

    char name[256];

    /* Look for velox.yaml in the config directories */
    file = open_config_file("velox.yaml");

    /* Nothing to do if there is no configuration file */
    if (file == NULL) return;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);

    if (!yaml_parser_load(&parser, &document))
    {
        fprintf(stderr, "Error parsing config file\n");
        goto cleanup;
    }

    /* The root node should be a mapping */
    map = yaml_document_get_root_node(&document);
    assert(map->type == YAML_MAPPING_NODE);

    /* For each key/value pair in the root mapping */
    for (pair = map->data.mapping.pairs.start; pair < map->data.mapping.pairs.top; ++pair)
    {
        key = yaml_document_get_node(&document, pair->key);
        value = yaml_document_get_node(&document, pair->value);

        assert(key->type == YAML_SCALAR_NODE);

        /* The module section */
        if (strcmp((const char const *) key->data.scalar.value, "modules") == 0)
        {
            yaml_node_item_t * module_item;
            yaml_node_t * node;

            assert(value->type == YAML_SEQUENCE_NODE);

            /* For each module */
            for (module_item = value->data.sequence.items.start;
                module_item < value->data.sequence.items.top;
                ++module_item)
            {
                node = yaml_document_get_node(&document, *module_item);

                load_module((const char const *) node->data.scalar.value);
            }
        }
        /* The border_width property */
        else if (strcmp((const char const *) key->data.scalar.value, "border_width") == 0)
        {
            assert(value->type == YAML_SCALAR_NODE);

            border_width = strtoul((const char const *) value->data.scalar.value, NULL, 10);
        }
    }

    yaml_document_delete(&document);

    /* While we still have documents to parse */
    while (yaml_parser_load(&parser, &document))
    {
        /* If the document contains no root node, we are at the end */
        if (yaml_document_get_root_node(&document) == NULL)
        {
            yaml_document_delete(&document);
            break;
        }

        sscanf((const char const *) yaml_document_get_root_node(&document)->tag, "!velox:%s", name);

        /* Configure the specified module with this YAML document */
        configure_module(name, &document);

        yaml_document_delete(&document);
    }

    cleanup:
        yaml_parser_delete(&parser);
        fclose(file);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

