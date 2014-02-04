/* velox: config.c
 *
 * Copyright (c) 2014 Michael Forney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "config.h"
#include "util.h"
#include "velox.h"

#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <swc.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

static CONFIG_GROUP(root);
struct wl_list * config_root = &root_group.group;

static uint32_t mod = SWC_MOD_LOGO;
static const char whitespace[] = " \t\n";

static bool parse_modifier(const char * string, uint32_t * modifier)
{
    if (strcmp(string, "mod") == 0)
        *modifier = mod;
    else if (strcmp(string, "ctrl") == 0)
        *modifier = SWC_MOD_CTRL;
    else if (strcmp(string, "alt") == 0)
        *modifier = SWC_MOD_ALT;
    else if (strcmp(string, "logo") == 0)
        *modifier = SWC_MOD_LOGO;
    else if (strcmp(string, "shift") == 0)
        *modifier = SWC_MOD_SHIFT;
    else if (strcmp(string, "any") == 0)
        *modifier = SWC_MOD_ANY;
    else
        return false;

    return true;
}

static bool mod_set(struct config_node * node, const char * value)
{
    return parse_modifier(value, &mod);
}

static CONFIG_PROPERTY(mod, &mod_set);

static bool section_match(const char * p, const char * q)
{
    for (; *p == *q && *p; ++p, ++q);
    return (*p == '.' || *p == '\0') && *q == '\0';
}

static struct config_node * lookup(char * identifier)
{
    struct config_node * group_node, * next_group_node, * node;
    char * section;

    for (group_node = &root_group; group_node; group_node = next_group_node)
    {
        section = identifier;
        identifier += strcspn(identifier, ".");
        next_group_node = NULL;

        wl_list_for_each(node, &group_node->group, link)
        {
            if (section_match(section, node->name))
            {
                if (node->type == CONFIG_NODE_TYPE_GROUP)
                {
                    if (*identifier == '\0')
                        return NULL;
                    next_group_node = node;
                    ++identifier;
                }
                else
                    return *identifier == '\0' ? node : NULL;

                break;
            }
        }
    }

    return NULL;
}

static bool handle_set(char * s)
{
    struct config_node * node;
    char * identifier, * value;

    identifier = s;
    s += strcspn(s, whitespace);

    if (*s == '\0')
        return false;

    *s++ = '\0';

    if (!(node = lookup(identifier)) || node->type != CONFIG_NODE_TYPE_PROPERTY)
    {
        fprintf(stderr, "Unknown identifier '%s'\n", identifier);
        return false;
    }

    s += strspn(s, whitespace);
    value = s;
    s += strcspn(s, whitespace);
    *s = '\0';
    node->property.set(node, value);

    return true;
}

struct spawn_action
{
    struct config_node node;
    const char * command;
};

static void spawn(struct config_node * node)
{
    struct spawn_action * action = wl_container_of(node, action, node);
    char * newline;

    if ((newline = strchr(action->command, '\n')))
        *newline = '\0';

    if (fork() == 0)
    {
        execl("/bin/sh", "sh", "-c", action->command, NULL);
        exit(EXIT_FAILURE);
    }
}

static struct config_node * spawn_action(char * command)
{
    struct spawn_action * action;

    if (!(action = malloc(sizeof *action)))
        return NULL;

    action->node.action.run = &spawn;
    action->command = strdup(command);

    return &action->node;
}

struct
{
    const char * name;
    struct config_node * (* create_action)(char * arguments);
} action_types[] = {
    { "spawn", &spawn_action }
};

static bool handle_action(char * s)
{
    char * identifier, * name, * type;
    unsigned index;
    struct config_node * node, * group_node;

    if (!(identifier = strtok_r(s, whitespace, &s)))
    {
        fprintf(stderr, "No action identifier specified\n");
        return false;
    }

    name = strrchr(identifier, '.');

    if (name)
    {
        *name++ = '\0';

        if (!(group_node = lookup(identifier)))
        {
            fprintf(stderr, "Invalid group identifier '%s'\n", identifier);
            return false;
        }
    }
    else
    {
        name = identifier;
        group_node = &root_group;
    }

    if (!(type = strtok_r(NULL, whitespace, &s)))
    {
        fprintf(stderr, "No action type specified\n");
        return false;
    }

    s += strspn(s, whitespace);

    for (index = 0; index < ARRAY_LENGTH(action_types); ++index)
    {
        if (strcmp(type, action_types[index].name) == 0)
        {
            if (!(node = action_types[index].create_action(s)))
            {
                fprintf(stderr, "Failed to create action '%s'\n", name);
                return false;
            }

            node->name = strdup(name);
            node->type = CONFIG_NODE_TYPE_ACTION;
            wl_list_insert(&group_node->group, &node->link);
        }
    }

    return true;
}

struct binding
{
    struct config_node * press, * release;
};

static void key_binding(void * data, uint32_t time, uint32_t value, uint32_t state)
{
    struct binding * binding = data;

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED && binding->press)
        binding->press->action.run(binding->press);
    else if (binding->release)
        binding->release->action.run(binding->release);
}

static void button_binding(void * data, uint32_t time, uint32_t value, uint32_t state)
{
    struct binding * binding = data;

    if (state == WL_POINTER_BUTTON_STATE_PRESSED && binding->press)
        binding->press->action.run(binding->press);
    else if (binding->release)
        binding->release->action.run(binding->release);
}

static void (* binding_handler[])(void *, uint32_t, uint32_t, uint32_t) = {
    [SWC_BINDING_KEY] = &key_binding,
    [SWC_BINDING_BUTTON] = &button_binding
};

static bool parse_key(char * s, uint32_t * value)
{
    *value = xkb_keysym_from_name(s, 0);

    if (*value == XKB_KEY_NoSymbol)
    {
        fprintf(stderr, "Invalid key '%s'\n", s);
        return false;
    }

    return true;
}

static bool parse_button(char * s, uint32_t * value)
{
    if (strcmp(s, "left") == 0)
        *value = BTN_LEFT;
    else if (strcmp(s, "right") == 0)
        *value = BTN_RIGHT;
    else if (strcmp(s, "middle") == 0)
        *value = BTN_MIDDLE;
    else if (strcmp(s, "side") == 0)
        *value = BTN_SIDE;
    else if (strcmp(s, "extra") == 0)
        *value = BTN_EXTRA;
    else
        return false;

    return true;
}

static bool (* parse_value[])(char *, uint32_t *) = {
    [SWC_BINDING_KEY] = &parse_key,
    [SWC_BINDING_BUTTON] = &parse_button
};

static bool parse_action(char * s, struct config_node ** node)
{
    if (*s == '\0')
    {
        *node = NULL;
        return true;
    }

    return (*node = lookup(s)) && (*node)->type == CONFIG_NODE_TYPE_ACTION;
}

static bool handle_binding(enum swc_binding_type type, char * s)
{
    char * value_string, * mod_string, * mods_string,
         * actions_string, * action_identifier;
    uint32_t value, mod, mods;
    struct binding * binding;

    if (!(binding = malloc(sizeof *binding)))
    {
        fprintf(stderr, "Failed to allocate binding\n");
        return false;
    }

    if (!(value_string = strtok_r(s, whitespace, &s)))
    {
        fprintf(stderr, "No key specified\n");
        return false;
    }

    if (!parse_value[type](value_string, &value))
        return false;

    if (!(mods_string = strtok_r(NULL, whitespace, &s)))
    {
        fprintf(stderr, "No modifiers specified\n");
        return false;
    }

    mods = 0;

    for (mod_string = strtok_r(mods_string, ",", &mods_string); mod_string;
         mod_string = strtok_r(NULL, ",", &mods_string))
    {
        if (!parse_modifier(mod_string, &mod))
        {
            fprintf(stderr, "Invalid modifier '%s'\n", mod_string);
            return false;
        }

        mods |= mod;
    }

    if (!(actions_string = strtok_r(NULL, whitespace, &s)))
    {
        fprintf(stderr, "No action specified\n");
        return false;
    }

    action_identifier = actions_string;
    actions_string += strcspn(actions_string, ":");

    if (*actions_string != '\0')
        *actions_string++ = '\0';

    /* Lookup press action (if present) */
    if (!parse_action(action_identifier, &binding->press))
    {
        fprintf(stderr, "Could not find action '%s'\n", action_identifier);
        return false;
    }

    action_identifier = actions_string;

    /* Lookup release action (if present) */
    if (!parse_action(action_identifier, &binding->release))
    {
        fprintf(stderr, "Could not find action '%s'\n", action_identifier);
        return false;
    }

    swc_add_binding(type, mods, value, binding_handler[type], binding);

    return true;
}

static bool handle_key(char * s)
{
    return handle_binding(SWC_BINDING_KEY, s);
}

static bool handle_button(char * s)
{
    return handle_binding(SWC_BINDING_BUTTON, s);
}

static const struct
{
    const char * name;
    bool (* handle)(char * arguments);
} commands[] = {
    { "set",    &handle_set },
    { "action", &handle_action },
    { "key",    &handle_key },
    { "button", &handle_button }
};

bool config_set_unsigned(unsigned * value, const char * string, int base)
{
    char * end;
    unsigned result;

    result = strtoul(string, &end, base);

    if (*end != '\0')
        return false;

    *value = result;

    return true;
}

static FILE * open_config()
{
    FILE * file;
    char path[256];

    snprintf(path, sizeof path, "%s/.velox.conf", getenv("HOME"));

    if ((file = fopen(path, "r")))
        goto found;

    strcpy(path, "/etc/velox.conf");

    if ((file = fopen(path, "r")))
        goto found;

    fprintf(stderr, "Couldn't find velox.conf\n");

    return NULL;

  found:
    fprintf(stderr, "Using config at '%s'\n", path);
    return file;
}

bool config_parse()
{
    FILE * file;
    char * line = NULL, * s, * command_name;
    unsigned index;
    bool handled;
    size_t size;

    wl_list_insert(&root_group.group, &mod_property.link);

    if (!(file = open_config()))
        goto error0;

    while (getline(&line, &size, file) != -1)
    {
        s = line + strspn(line, whitespace);

        if (*s == '#' || *s == '\0')
            continue;

        command_name = s;
        s += strcspn(s, whitespace);

        if (!*s)
            goto error1;

        *s++ = '\0';
        handled = false;

        for (index = 0; index < ARRAY_LENGTH(commands); ++index)
        {
            if (strcmp(commands[index].name, command_name) == 0)
            {
                if (!commands[index].handle(s))
                    goto error1;
                handled = true;
                break;
            }
        }

        if (!handled)
            goto error1;
    }

    return true;

  error1:
    fclose(file);
  error0:
    return false;
}

