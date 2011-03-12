/* velox: modules/pulseaudio.c
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

#include <stdio.h>
#include <assert.h>
#include <yaml.h>
#include <pulse/pulseaudio.h>

#include <velox/velox.h>
#include <velox/module.h>
#include <velox/debug.h>


const char name[] = "pulseaudio";

//const uint8_t max_sinks = 16;
#define max_sinks 16
const pa_volume_t volume_change = 2 * (PA_VOLUME_NORM - PA_VOLUME_MUTED) / 100;

struct sink_info
{
    bool mute;
    pa_cvolume volume;
};

static struct sink_info sinks[max_sinks];
static uint8_t sinks_size = 0;

static bool querying_sinks = false;
static pthread_mutex_t sinks_mutex;

static pa_threaded_mainloop * mainloop;
static pa_context * context;

/* Configuration parameters */
static uint8_t sink_index = 0;

void sink_callback(pa_context * c, const pa_sink_info * info, int eol, void * userdata)
{
    DEBUG_ENTER

    if (eol > 0)
    {
        if (sink_index >= sinks_size) sink_index = 0;
        pthread_mutex_unlock(&sinks_mutex);
    }
    else
    {
        DEBUG_PRINT("sink index: %u\n", info->index);
        DEBUG_PRINT("sink description: %s\n", info->description);

        if (info->index < max_sinks)
        {
            ++sinks_size;
            sinks[info->index] = (struct sink_info) { info->mute, info->volume };
        }
    }
}

void subscribe_callback(pa_context * c, pa_subscription_event_type_t type,
    uint32_t index, void * userdata)
{
    pa_operation * op;

    DEBUG_ENTER

    switch (type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
        case PA_SUBSCRIPTION_EVENT_SINK:
            DEBUG_PRINT("sink event\n")

            if (!(op = pa_context_get_sink_info_by_index(context, index,
                sink_callback, NULL)))
            {
                DEBUG_PRINT("failed to get sink info\n");
                break;
            }

            pa_operation_unref(op);

            break;

        default:
            break;
    }
}

void state_callback(pa_context * c, void * userdata)
{
    pa_context_state_t state;

    DEBUG_ENTER

    state = pa_context_get_state(context);

    DEBUG_PRINT("state: %u\n", state)

    switch (state)
    {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
        {
            pa_operation * op;

            DEBUG_PRINT("ready\n")

            if (!(op = pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_SINK,
                NULL, NULL)))
            {
                DEBUG_PRINT("failed to subscribe\n")
                break;
            }

            pa_operation_unref(op);

            sinks_size = 0;
            pthread_mutex_lock(&sinks_mutex);

            if (!(op = pa_context_get_sink_info_list(context, sink_callback,
                NULL)))
            {
                DEBUG_PRINT("failed to get sink list\n");
                break;
            }

            pa_operation_unref(op);

            break;
        }

        case PA_CONTEXT_FAILED:
            break;

        case PA_CONTEXT_TERMINATED:
        default:
            break;
    }
}

static void cycle(union velox_argument argument)
{
    sink_index = ++sink_index % sinks_size;

    DEBUG_PRINT("sink_index: %u\n", sink_index)
}

static void mute(union velox_argument argument)
{
    pthread_mutex_lock(&sinks_mutex);
    pa_threaded_mainloop_lock(mainloop);

    pa_context_set_sink_mute_by_index(context, sink_index,
        !sinks[sink_index].mute, NULL, NULL);

    pa_threaded_mainloop_unlock(mainloop);
    pthread_mutex_unlock(&sinks_mutex);
}

static void change_volume(union velox_argument argument)
{
    bool increase = argument.uint8;
    pa_cvolume * cvolume;
    pa_volume_t volume;

    pthread_mutex_lock(&sinks_mutex);
    pa_threaded_mainloop_lock(mainloop);

    cvolume = &sinks[sink_index].volume;
    volume = pa_cvolume_avg(cvolume);

    DEBUG_PRINT("old volume: %u\n", volume);

    if (increase && volume > PA_VOLUME_NORM - volume_change)
    {
        volume = PA_VOLUME_NORM;
    }
    else if (!increase && volume < PA_VOLUME_MUTED + volume_change)
    {
        volume = PA_VOLUME_MUTED;
    }
    else volume += increase ? volume_change : -volume_change;

    DEBUG_PRINT("new volume: %u\n", volume);

    pa_cvolume_set(cvolume, cvolume->channels, volume);

    pa_context_set_sink_volume_by_index(context, sink_index,
        cvolume, NULL, NULL);

    pa_threaded_mainloop_unlock(mainloop);
    pthread_mutex_unlock(&sinks_mutex);
}

void configure(yaml_document_t * document)
{
    yaml_node_t * map;
    yaml_node_pair_t * pair;

    yaml_node_t * key, * value;

    printf("PulseAudio: Loading configuration...");

    /*
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
    }
    */

    printf("done\n");
}

bool setup()
{
    printf("PulseAudio: Initializing...");

    pthread_mutex_init(&sinks_mutex, NULL);

    mainloop = pa_threaded_mainloop_new();
    context = pa_context_new(pa_threaded_mainloop_get_api(mainloop),
        "Velox Volume Control");
    pa_context_set_state_callback(context, state_callback, NULL);
    pa_context_set_subscribe_callback(context, subscribe_callback, NULL);
    pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);

    MODULE_KEY_BINDING(cycle, no_argument);
    MODULE_KEY_BINDING(mute, no_argument);

    add_key_binding(name, "raise_volume", change_volume, uint8_argument(true));
    add_key_binding(name, "lower_volume", change_volume, uint8_argument(false));

    pa_threaded_mainloop_start(mainloop);

    printf("done\n");

    return true;
}

void cleanup()
{
    printf("PulseAudio: Cleaning up...");

    pa_context_disconnect(context);
    pa_context_unref(context);
    pa_threaded_mainloop_stop(mainloop);
    pa_threaded_mainloop_free(mainloop);

    pthread_mutex_destroy(&sinks_mutex);

    printf("done\n");
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

