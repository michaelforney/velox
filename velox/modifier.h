/* velox: velox/modifier.h
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

#ifndef VELOX_MODIFIER_H
#define VELOX_MODIFIER_H

#include <yaml.h>
#include <assert.h>

/* Macros */
#define CLEAN_MASK(mask) (mask & ~(MOD_MASK_NUMLOCK | XCB_MOD_MASK_LOCK))

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

static uint16_t parse_modifiers(yaml_document_t * document, yaml_node_t * node)
{
    yaml_node_item_t * item;
    yaml_node_t * mod_node;

    uint16_t modifiers = 0;

    assert(node->type == YAML_SEQUENCE_NODE);

    for (item = node->data.sequence.items.start;
        item < node->data.sequence.items.top;
        ++item)
    {
        mod_node = yaml_document_get_node(document, *item);
        assert(mod_node->type == YAML_SCALAR_NODE);
        modifiers |= modifier_value((const char *) mod_node->data.scalar.value);
    }

    return modifiers;
}

/* Key binding constants */
#define MOD_MASK_NUMLOCK XCB_MOD_MASK_2
static const uint16_t extra_modifiers[] = {
    0,
    MOD_MASK_NUMLOCK,
    XCB_MOD_MASK_LOCK,
    MOD_MASK_NUMLOCK | XCB_MOD_MASK_LOCK
};

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

