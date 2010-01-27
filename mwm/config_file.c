/* mwm: mwm/config_file.c
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
 *
 * This file is a part of mwm.
 *
 * mwm is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * mwm is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with mwm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <yaml.h>
#include <assert.h>

#include "config_file.h"

#include "module-private.h"

FILE * open_config_file(const char * name)
{
    FILE * file;
    char path[1024];

    snprintf(path, sizeof(path), "%s/.mwm/%s", getenv("HOME"), name);
    if ((file = fopen(path, "r")) != NULL) return file;

    snprintf(path, sizeof(path), "/etc/mwm/%s", name);
    if ((file = fopen(path, "r")) != NULL) return file;

    return NULL;
}

void parse_config()
{
    FILE * file;

    yaml_parser_t parser;
    yaml_document_t document;

    file = open_config_file("mwm.yaml");

    if (file == NULL) return;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);

    if (yaml_parser_load(&parser, &document))
    {
        yaml_node_t * map;
        yaml_node_pair_t * pair;

        yaml_node_t * key, * value;

        map = document.nodes.start;
        assert(map->type == YAML_MAPPING_NODE);

        for (pair = map->data.mapping.pairs.start; pair < map->data.mapping.pairs.top; ++pair)
        {
            key = yaml_document_get_node(&document, pair->key);
            value = yaml_document_get_node(&document, pair->value);

            assert(key->type == YAML_SCALAR_NODE);

            if (strcmp((const char const *) key->data.scalar.value, "modules") == 0)
            {
                yaml_node_item_t * module_item;
                yaml_node_t * node;

                assert(value->type == YAML_SEQUENCE_NODE);

                for (module_item = value->data.sequence.items.start; module_item < value->data.sequence.items.top; ++module_item)
                {
                    node = yaml_document_get_node(&document, *module_item);

                    load_module((const char const *) node->data.scalar.value);
                }
            }
        }

        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
    }
    else
    {
        printf("Error parsing document\n");
    }

    fclose(file);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

