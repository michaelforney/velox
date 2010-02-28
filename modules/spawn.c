/* velox: modules/spawn.c
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
#include <assert.h>
#include <yaml.h>

#include <velox/velox.h>
#include <velox/module.h>

const char name[] = "spawn";

static const char *** commands = NULL;

static void spawn_command(void * arg);

void configure(yaml_document_t * document)
{
    yaml_node_t * sequence;
    yaml_node_item_t * item;

    yaml_node_t * node;

    printf("Spawn: Loading configuration...");

    sequence = yaml_document_get_root_node(document);
    assert(sequence->type == YAML_SEQUENCE_NODE);

    commands = (const char ***) malloc((sequence->data.sequence.items.top - sequence->data.sequence.items.start + 1) * sizeof(const char ***));
    commands[
        sequence->data.sequence.items.top -
        sequence->data.sequence.items.start
    ] = NULL;

    for (item = sequence->data.sequence.items.start;
        item < sequence->data.sequence.items.top;
        ++item)
    {
        yaml_node_pair_t * pair;
        yaml_node_t * key, * value;
        const char ** command;
        const char * binding;

        node = yaml_document_get_node(document, *item);

        assert(node->type == YAML_MAPPING_NODE);

        for (pair = node->data.mapping.pairs.start;
            pair < node->data.mapping.pairs.top;
            ++pair)
        {
            key = yaml_document_get_node(document, pair->key);
            value = yaml_document_get_node(document, pair->value);

            assert(key->type == YAML_SCALAR_NODE);

            if (strcmp((const char const *) key->data.scalar.value, "command") == 0)
            {
                uint16_t command_length;

                yaml_node_item_t * command_item;
                yaml_node_t * command_node;

                assert(value->type == YAML_SEQUENCE_NODE);

                command = (const char **) malloc((value->data.sequence.items.top - value->data.sequence.items.start + 1) * sizeof(const char **));
                command[
                    value->data.sequence.items.top -
                    value->data.sequence.items.start
                ] = NULL;

                for (command_item = value->data.sequence.items.start;
                    command_item < value->data.sequence.items.top;
                    ++command_item)
                {
                    command_node = yaml_document_get_node(document, *command_item);

                    assert(command_node->type == YAML_SCALAR_NODE);

                    command[command_item - value->data.sequence.items.start] = strdup((const char const *) command_node->data.scalar.value);
                }

                commands[item - sequence->data.sequence.items.start] = command;
            }
            else if (strcmp((const char const *) key->data.scalar.value, "binding") == 0)
            {
                assert(value->type == YAML_SCALAR_NODE);

                binding = (const char const *) value->data.scalar.value;
            }
        }

        add_configured_key_binding(name, binding, &spawn_command, command);
    }

    printf("done\n");
}

bool setup()
{
    printf("Spawn: Initializing...");
    printf("done\n");

    return true;
}

void cleanup()
{
    const char *** iterator;
    const char ** command_iterator;

    printf("Spawn: Cleaning up...");

    for (iterator = commands; *iterator != NULL; ++iterator)
    {
        for (command_iterator = *iterator;
            *command_iterator != NULL;
            ++command_iterator)
        {
            free((void *) *command_iterator);
        }

        free(*iterator);
    }

    free(commands);

    printf("done\n");
}

void spawn_command(void * arg)
{
    printf("spawn_command: %s\n", ((char * const *) arg)[0]);
    spawn((char * const *) arg);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

