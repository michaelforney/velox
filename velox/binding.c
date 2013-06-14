/* velox: velox/binding.c
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

#include <yaml.h>
#include <assert.h>
#include <xkbcommon/xkbcommon.h>

#include "hashtable.h"
#include "velox.h"
#include "config_file.h"
#include "modifier.h"
#include "hook.h"
#include "debug.h"

#include "x11/keyboard_mapping.h"

#include "binding-private.h"

/* Helper macros */
#define STRING_SYMBOL(name) #name, &name

DEFINE_HASHTABLE(velox_bindable_hashtable, const char *, struct velox_vector *);

static void parse_button(yaml_node_t * node, struct velox_bindable * bindable);
static void parse_key(yaml_node_t * node, struct velox_bindable * bindable);

struct velox_vector key_bindings;
struct velox_vector button_bindings;

struct velox_bindable_hashtable configured_keys;
struct velox_bindable_hashtable configured_buttons;

static void __attribute__((constructor)) initialize_bindings()
{
    hashtable_initialize(&configured_keys, 512, &sdbm_hash);
    hashtable_initialize(&configured_buttons, 512, &sdbm_hash);

    vector_initialize(&key_bindings, sizeof(struct velox_binding), 128);
    vector_initialize(&button_bindings, sizeof(struct velox_binding), 16);
}

static void __attribute__((destructor)) free_bindings()
{
    hashtable_free(&configured_keys);
    hashtable_free(&configured_buttons);
}

static void parse_button(yaml_node_t * node, struct velox_bindable * bindable)
{
    assert(node->type == YAML_SCALAR_NODE);

    if (strcmp((const char *) node->data.scalar.value, "any")  == 0)
    {
        bindable->pressable.button = XCB_BUTTON_INDEX_ANY;
    }
    else
    {
        bindable->pressable.button = strtoul((const char *)
            node->data.scalar.value, NULL, 10);
    }
}

static void parse_key(yaml_node_t * node, struct velox_bindable * bindable)
{
    assert(node->type == YAML_SCALAR_NODE);

    bindable->pressable.key = xkb_keysym_from_name((const char *)
        node->data.scalar.value, 0);
}

static void setup_configured_bindings(const char * filename,
    void (* parse_function)(yaml_node_t * node, struct velox_bindable * bindable),
    struct velox_bindable_hashtable * configured_bindings)
{
    FILE * file;
    char identifier[256];
    struct velox_vector * bindable_vector;

    yaml_parser_t parser;
    yaml_document_t document;

    /* Look for and open keys.yaml in the standard configuration directories */
    file = open_config_file(filename);

    assert(file);

    printf("\n** Loading configured bindings from %s **\n", filename);

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    if (yaml_parser_load(&parser, &document))
    {
        yaml_node_t * root_map;

        yaml_node_pair_t * root_pair;
        yaml_node_t * root_key, * root_value;

        root_map = document.nodes.start;
        assert(root_map->type == YAML_MAPPING_NODE);

        /* For each key section */
        for (root_pair = root_map->data.mapping.pairs.start;
            root_pair < root_map->data.mapping.pairs.top;
            ++root_pair)
        {
            root_key = yaml_document_get_node(&document, root_pair->key);
            root_value = yaml_document_get_node(&document, root_pair->value);

            assert(root_key->type == YAML_SCALAR_NODE);
            assert(root_value->type == YAML_MAPPING_NODE);

            yaml_node_pair_t * set_pair;
            yaml_node_t * set_key, * set_value;

            /* For each set of bindings */
            for (set_pair = root_value->data.mapping.pairs.start;
                set_pair < root_value->data.mapping.pairs.top;
                ++set_pair)
            {
                yaml_node_item_t * mapping_item;
                yaml_node_t * mapping_node;

                uint32_t key_index;

                set_key = yaml_document_get_node(&document, set_pair->key);
                set_value = yaml_document_get_node(&document, set_pair->value);

                assert(set_key->type == YAML_SCALAR_NODE);
                assert(set_value->type == YAML_SEQUENCE_NODE);

                bindable_vector = malloc(sizeof(struct velox_vector));
                vector_initialize(bindable_vector, sizeof(struct velox_bindable),
                    set_value->data.sequence.items.top
                    - set_value->data.sequence.items.start);

                snprintf(identifier, sizeof(identifier), "%s:%s",
                    root_key->data.scalar.value,
                    set_key->data.scalar.value
                );

                /* For each mapping */
                for (mapping_item = set_value->data.sequence.items.start, key_index = 0;
                    mapping_item < set_value->data.sequence.items.top;
                    ++mapping_item, ++key_index)
                {
                    yaml_node_pair_t * binding_pair;
                    yaml_node_t * binding_key, * binding_value;
                    struct velox_bindable * bindable;

                    mapping_node = yaml_document_get_node(&document, *mapping_item);

                    assert(mapping_node->type == YAML_MAPPING_NODE);

                    bindable = vector_add(bindable_vector);

                    /* Identify key */
                    for (binding_pair = mapping_node->data.mapping.pairs.start;
                        binding_pair < mapping_node->data.mapping.pairs.top;
                        ++binding_pair)
                    {
                        binding_key = yaml_document_get_node(&document, binding_pair->key);
                        binding_value = yaml_document_get_node(&document, binding_pair->value);

                        assert(binding_key->type == YAML_SCALAR_NODE);

                        if (strcmp((const char *) binding_key->data.scalar.value, "mod") == 0)
                        {
                            bindable->modifiers = parse_modifiers(&document, binding_value);
                        }
                        else if (strcmp((const char *) binding_key->data.scalar.value, "value") == 0)
                        {
                            parse_function(binding_value, bindable);
                        }
                    }
                }

                hashtable_insert(configured_bindings, identifier, bindable_vector);
            }
        }

        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
    }
    else fprintf(stderr, "YAML error: %s\n", parser.problem);

    fclose(file);
}

static void add_binding(struct velox_bindable_hashtable * configured_bindables,
    struct velox_vector * bindings, const char * group, const char * name,
    velox_function_t function, union velox_argument arg)
{
    struct velox_vector * bindable_vector;
    struct velox_bindable * bindable;
    char identifier[strlen(group) + strlen(name) + 1];

    sprintf(identifier, "%s:%s", group, name);

    /* Lookup the list of bindables associated with the given identifier in the
     * configured_bindings table */
    bindable_vector = hashtable_lookup(configured_bindables, identifier);

    assert(bindable_vector);

    vector_for_each(bindable_vector, bindable)
    {
        struct velox_binding * binding;

        binding = vector_add(bindings);
        binding->bindable = *bindable;
        binding->function = function;
        binding->arg = arg;
    }
}

static void setup_key_bindings()
{
    setup_configured_bindings("keys.yaml", &parse_key, &configured_keys);

    /* Window focus */
    add_key_binding("velox", STRING_SYMBOL(focus_next), no_argument);
    add_key_binding("velox", STRING_SYMBOL(focus_previous), no_argument);
    add_key_binding("velox", STRING_SYMBOL(move_next), no_argument);
    add_key_binding("velox", STRING_SYMBOL(move_previous), no_argument);

    add_key_binding("velox", STRING_SYMBOL(toggle_focus_type), no_argument);

    /* Window operations */
    add_key_binding("velox", STRING_SYMBOL(kill_focused_window), no_argument);
    add_key_binding("velox", STRING_SYMBOL(toggle_floating), no_argument);

    /* Layout control */
    add_key_binding("velox", STRING_SYMBOL(next_layout), no_argument);
    add_key_binding("velox", STRING_SYMBOL(previous_layout), no_argument);

    /* Quit */
    add_key_binding("velox", STRING_SYMBOL(quit), no_argument);
}

static void setup_button_bindings()
{
    setup_configured_bindings("buttons.yaml", &parse_button, &configured_buttons);

    /* Floating windows */
    add_button_binding("velox", STRING_SYMBOL(move_float));
    add_button_binding("velox", STRING_SYMBOL(resize_float));

    add_button_binding("velox", STRING_SYMBOL(next_workspace));
    add_button_binding("velox", STRING_SYMBOL(previous_workspace));
}

void add_key_binding(const char * group, const char * name,
    velox_function_t function, union velox_argument arg)
{
    add_binding(&configured_keys, &key_bindings, group, name, function, arg);
}

void add_button_binding(const char * group, const char * name,
    velox_function_t function)
{
    add_binding(&configured_buttons, &button_bindings, group, name, function, no_argument);
}

void grab_keys(union velox_argument argument)
{
    xcb_keycode_t * keycode;
    struct velox_binding * binding;
    uint8_t modifier_index;
    uint8_t extra_modifiers_length = sizeof(extra_modifiers) / sizeof(uint16_t);

    DEBUG_ENTER

    xcb_ungrab_key(c, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);

    vector_for_each(&key_bindings, binding)
    {
        xcb_keycode_t * keycodes = xcb_key_symbols_get_keycode(keyboard_mapping,
            binding->bindable.pressable.key);

        if (!keycodes) continue;

        keycode = keycodes;

        while (*keycode != XCB_NO_SYMBOL)
        {
            for (modifier_index = 0; modifier_index < extra_modifiers_length;
                ++modifier_index)
            {
                xcb_grab_key(c, true, screen->root,
                    binding->bindable.modifiers | extra_modifiers[modifier_index],
                    *keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
            }

            ++keycode;
        }

        free(keycodes);
    }

    xcb_flush(c);
}

void setup_bindings()
{
    setup_key_bindings();
    setup_button_bindings();

    /* Add binding related hooks */
    add_hook(&grab_keys, VELOX_HOOK_KEYBOARD_MAPPING_CHANGED);
}

void cleanup_bindings()
{
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

