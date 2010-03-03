/* velox: velox/keybinding.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
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
#include <stdlib.h>
#include <yaml.h>
#include <assert.h>
#include <X11/Xlib.h> // For XStringToKeysym

#include "keybinding.h"
#include "tag.h"
#include "config_file.h"
#include "velox.h"
#include "hashtable.h"
#include "debug.h"

/* Helper macros */
#define STRING_SYMBOL(name) #name, &name

/* Define container structures */
struct key_list
{
    struct velox_key * keys;
    uint32_t length;
};

DEFINE_HASHTABLE(key_hashtable, const char *, struct key_list *);

/* Containers */
LIST_HEAD(key_bindings);
struct key_hashtable configured_keys;

/* X variables */
xcb_get_keyboard_mapping_reply_t * keyboard_mapping;

/* Key binding constants */
const uint16_t mod_mask_numlock = XCB_MOD_MASK_2;

/* Constructors and destructors */
void __attribute__((constructor)) initialize_keys()
{
    hashtable_initialize(&configured_keys, 512, &sdbm_hash);
}

void __attribute__((destructor)) free_keys()
{
    hashtable_free(&configured_keys);
}

/* Static functions */
static inline uint16_t modifier_value(const char * name)
{
    if (strcmp(name, "mod_shift") == 0)         return XCB_MOD_MASK_SHIFT;
    else if (strcmp(name, "mod_lock") == 0)     return XCB_MOD_MASK_LOCK;
    else if (strcmp(name, "mod_control") == 0)  return XCB_MOD_MASK_CONTROL;
    else if (strcmp(name, "mod_1") == 0)        return XCB_MOD_MASK_1;
    else if (strcmp(name, "mod_2") == 0)        return XCB_MOD_MASK_2;
    else if (strcmp(name, "mod_3") == 0)        return XCB_MOD_MASK_3;
    else if (strcmp(name, "mod_4") == 0)        return XCB_MOD_MASK_4;
    else if (strcmp(name, "mod_5") == 0)        return XCB_MOD_MASK_5;
    else if (strcmp(name, "mod_any") == 0)      return XCB_MOD_MASK_ANY;

    return 0;
}

static void setup_configured_keys()
{
    FILE * file;
    char identifier[256];
    struct key_list * key_list;

    yaml_parser_t parser;
    yaml_document_t document;

    /* Look for and open keys.yaml in the standard configuration directories */
    file = open_config_file("keys.yaml");

    assert(file);

    printf("\n** Loading Configured Key Bindings **\n");

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

            /* For each set of key mappings */
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

                key_list = (struct key_list *) malloc(sizeof(struct key_list));
                key_list->keys = (struct velox_key *) malloc(
                    (set_value->data.sequence.items.top -
                        set_value->data.sequence.items.start)
                    * sizeof(struct velox_key));
                key_list->length = set_value->data.sequence.items.top -
                    set_value->data.sequence.items.start;

                snprintf(identifier, sizeof(identifier), "%s:%s",
                    root_key->data.scalar.value,
                    set_key->data.scalar.value
                );

                /* For each key mapping */
                for (mapping_item = set_value->data.sequence.items.start, key_index = 0;
                    mapping_item < set_value->data.sequence.items.top;
                    ++mapping_item, ++key_index)
                {
                    yaml_node_pair_t * key_pair;
                    yaml_node_t * key_key, * key_value;

                    mapping_node = yaml_document_get_node(&document, *mapping_item);

                    assert(mapping_node->type == YAML_MAPPING_NODE);

                    /* Identify key */
                    for (key_pair = mapping_node->data.mapping.pairs.start;
                        key_pair < mapping_node->data.mapping.pairs.top;
                        ++key_pair)
                    {
                        key_key = yaml_document_get_node(&document, key_pair->key);
                        key_value = yaml_document_get_node(&document, key_pair->value);

                        assert(key_key->type == YAML_SCALAR_NODE);

                        if (strcmp((const char const *) key_key->data.scalar.value, "mod") == 0)
                        {
                            yaml_node_item_t * mod_item;
                            yaml_node_t * mod_node;

                            key_list->keys[key_index].modifiers = 0;

                            assert(key_value->type == YAML_SEQUENCE_NODE);

                            for (mod_item = key_value->data.sequence.items.start;
                                mod_item < key_value->data.sequence.items.top;
                                ++mod_item)
                            {
                                mod_node = yaml_document_get_node(&document, *mod_item);
                                assert(mod_node->type == YAML_SCALAR_NODE);
                                key_list->keys[key_index].modifiers |= modifier_value(
                                    (const char const *) mod_node->data.scalar.value);
                            }
                        }
                        else if (strcmp((const char const *) key_key->data.scalar.value, "key") == 0)
                        {
                            assert(key_value->type == YAML_SCALAR_NODE);
                            key_list->keys[key_index].keysym = XStringToKeysym((const char const *) key_value->data.scalar.value);
                        }
                    }

                    printf("%s (modifiers: %i, keysym: %x)\n",
                        identifier, key_list->keys[key_index].modifiers,
                        key_list->keys[key_index].keysym);
                }

                hashtable_insert(&configured_keys, identifier, key_list);
            }
        }

        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
    }
    else fprintf(stderr, "YAML error: %s\n", parser.problem);

    fclose(file);
}

/* Setup and cleanup functions */
void setup_key_bindings()
{
    setup_configured_keys();

    /* Window focus */
    add_configured_key_binding("velox", STRING_SYMBOL(focus_next), NULL);
    add_configured_key_binding("velox", STRING_SYMBOL(focus_previous), NULL);
    add_configured_key_binding("velox", STRING_SYMBOL(move_next), NULL);
    add_configured_key_binding("velox", STRING_SYMBOL(move_previous), NULL);

    add_configured_key_binding("velox", STRING_SYMBOL(toggle_focus_type), NULL);

    /* Window operations */
    add_configured_key_binding("velox", STRING_SYMBOL(kill_focused_window), NULL);

    /* Layout control */
    add_configured_key_binding("velox", STRING_SYMBOL(next_layout), NULL);
    add_configured_key_binding("velox", STRING_SYMBOL(previous_layout), NULL);

    /* Quit */
    add_configured_key_binding("velox", STRING_SYMBOL(quit), NULL);
}

void cleanup_key_bindings()
{
    struct velox_key_binding_entry * entry, * n;

    list_for_each_entry_safe(entry, n, &key_bindings, head)
    {
        free(entry->key_binding);
        free(entry);
    }
}

/* Private functions */
void grab_keys(xcb_keycode_t min_keycode, xcb_keycode_t max_keycode)
{
    xcb_get_keyboard_mapping_cookie_t keyboard_mapping_cookie;
    xcb_keysym_t * keysyms;
    struct velox_key_binding_entry * entry;
    uint16_t keysym_index;
    uint16_t extra_modifier_index;
    uint16_t extra_modifiers[] = {
        0,
        mod_mask_numlock,
        XCB_MOD_MASK_LOCK,
        mod_mask_numlock | XCB_MOD_MASK_LOCK
    };
    uint16_t extra_modifiers_count = sizeof(extra_modifiers) / sizeof(uint16_t);

    DEBUG_ENTER

    keyboard_mapping_cookie = xcb_get_keyboard_mapping(c, min_keycode, max_keycode - min_keycode + 1);

    xcb_ungrab_key(c, XCB_GRAB_ANY, root, XCB_MOD_MASK_ANY);

    free(keyboard_mapping);
    keyboard_mapping = xcb_get_keyboard_mapping_reply(c, keyboard_mapping_cookie, NULL);
    keysyms = xcb_get_keyboard_mapping_keysyms(keyboard_mapping);
    list_for_each_entry(entry, &key_bindings, head)
    {
        for (keysym_index = 0; keysym_index < xcb_get_keyboard_mapping_keysyms_length(keyboard_mapping); keysym_index++)
        {
            if (keysyms[keysym_index] == entry->key_binding->key.keysym)
            {
                entry->key_binding->keycode = min_keycode + (keysym_index / keyboard_mapping->keysyms_per_keycode);
                break;
            }
        }

        for (extra_modifier_index = 0; extra_modifier_index < extra_modifiers_count; extra_modifier_index++)
        {
            xcb_grab_key(c, true, root,
                entry->key_binding->key.modifiers | extra_modifiers[extra_modifier_index],
                entry->key_binding->keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
            );
        }
    }

    xcb_flush(c);
}

/* Public functions */
void add_key_binding(struct velox_key * key, void (* function)(void * arg), void * arg)
{
    struct velox_key_binding * binding;
    struct velox_key_binding_entry * entry;

    /* Allocate a new key binding and set its values */
    binding = (struct velox_key_binding *) malloc(sizeof(struct velox_key_binding));
    binding->key = *key;
    binding->keycode = 0;
    binding->function = function;
    binding->arg = arg;

    entry = (struct velox_key_binding_entry *) malloc(sizeof(struct velox_key_binding_entry));
    entry->key_binding = binding;

    list_add(&entry->head, &key_bindings);
}

void add_configured_key_binding(const char * group, const char * name,
    void (* function)(void * arg), void * arg)
{
    struct key_list * key_list;
    uint32_t key_index;
    char identifier[strlen(group) + strlen(name) + 1];

    sprintf(identifier, "%s:%s", group, name);

    /* Lookup the list of keys associated with that binding in the
     * configured_keys table */
    key_list = hashtable_lookup(&configured_keys, identifier);

    assert(key_list);

    for (key_index = 0; key_index < key_list->length; ++key_index)
    {
        add_key_binding(&key_list->keys[key_index], function, arg);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

