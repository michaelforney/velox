/* velox: modules/alsa.c
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
#include <asoundlib.h>
#include <mixer.h>

#include <velox/velox.h>
#include <velox/module.h>

#define VOLUME_CHANGE 5

const char name[] = "alsa";

static snd_mixer_t * mixer;
static snd_mixer_elem_t * mixer_elem;

/* Configuration parameters */
static const char * mixer_card = "default";
static const char * mixer_name = "Master";
static uint16_t mixer_index = 0;

static void mute(void * arg);
static void raise_volume(void * arg);
static void lower_volume(void * arg);

void configure(yaml_document_t * document)
{
    yaml_node_t * map;
    yaml_node_pair_t * pair;

    yaml_node_t * key, * value;

    printf("Alsa: Loading configuration...");

    map = yaml_document_get_root_node(document);
    assert(map->type == YAML_MAPPING_NODE);

    for (pair = map->data.mapping.pairs.start;
        pair < map->data.mapping.pairs.top;
        ++pair)
    {
        key = yaml_document_get_node(document, pair->key);
        value = yaml_document_get_node(document, pair->value);

        assert(key->type == YAML_SCALAR_NODE);
        assert(value->type == YAML_SCALAR_NODE);

        if (strcmp((const char const *) key->data.scalar.value, "card") == 0)
        {
            mixer_card = strdup((const char const *) value->data.scalar.value);
        }
        else if (strcmp((const char const *) key->data.scalar.value, "name") == 0)
        {
            mixer_name = strdup((const char const *) value->data.scalar.value);
        }
        else if (strcmp((const char const *) key->data.scalar.value, "index") == 0)
        {
            mixer_index = strtoul((const char const *) value->data.scalar.value, NULL, 10);
        }
    }

    printf("done\n");
}

bool setup()
{
    snd_mixer_selem_id_t * sid;

    printf("Alsa: Initializing...");

    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, mixer_card);
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_alloca(&sid);

    snd_mixer_selem_id_set_name(sid, mixer_name);
    snd_mixer_selem_id_set_index(sid, mixer_index);

    mixer_elem = snd_mixer_find_selem(mixer, sid);

    MODULE_KEY_BINDING(mute, NULL);
    MODULE_KEY_BINDING(raise_volume, NULL);
    MODULE_KEY_BINDING(lower_volume, NULL);

    printf("done\n");

    return true;
}

void cleanup()
{
    printf("Alsa: Cleaning up...");

    snd_mixer_close(mixer);

    printf("done\n");
}

void mute(void * arg)
{
    int muted;

    if (!snd_mixer_selem_has_playback_switch(mixer_elem)) return;

    /* If the mixer mutes all channels together */
    if (snd_mixer_selem_has_playback_switch_joined(mixer_elem))
    {
        snd_mixer_selem_get_playback_switch(mixer_elem, 0, &muted);
        snd_mixer_selem_set_playback_switch(mixer_elem, 0, !muted);
    }
    /* If the mixer mutes by channel */
    else
    {
        uint16_t channel;

        for (channel = 0; channel < SND_MIXER_SCHN_LAST; ++channel)
        {
            if (snd_mixer_selem_has_playback_channel(mixer_elem, channel))
            {
                snd_mixer_selem_get_playback_switch(mixer_elem, channel, &muted);
                snd_mixer_selem_set_playback_switch(mixer_elem, channel, !muted);
            }
        }
    }
}

static void change_volume(int16_t amount)
{
    long volume_min, volume_max;
    long volume;
    int32_t mixer_amount;

    if (!snd_mixer_selem_has_playback_volume(mixer_elem)) return;

    snd_mixer_selem_get_playback_volume_range(mixer_elem, &volume_min, &volume_max);

    mixer_amount = amount * (volume_max - volume_min) / 100;

    /* If the mixer mutes all channels together */
    if (snd_mixer_selem_has_playback_volume_joined(mixer_elem))
    {
        snd_mixer_selem_get_playback_volume(mixer_elem, 0, &volume);
        snd_mixer_selem_set_playback_volume(mixer_elem, 0, MAX(MIN(volume + mixer_amount, 100), 0));
    }
    /* If the mixer mutes by channel */
    else
    {
        uint16_t channel;

        for (channel = 0; channel < SND_MIXER_SCHN_LAST; ++channel)
        {
            if (snd_mixer_selem_has_playback_channel(mixer_elem, channel))
            {
                snd_mixer_selem_get_playback_volume(mixer_elem, channel, &volume);
                snd_mixer_selem_set_playback_volume(mixer_elem, channel, MAX(MIN(volume + mixer_amount, 100), 0));
            }
        }
    }
}

static void raise_volume(void * arg)
{
    printf("VOLUME_CHANGE: %u\n", VOLUME_CHANGE);
    change_volume(VOLUME_CHANGE);
}

static void lower_volume(void * arg)
{
    change_volume(-VOLUME_CHANGE);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

