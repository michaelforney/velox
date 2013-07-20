/* velox: modules/bar.c
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
#include <yaml.h>
#include <xcb/xcb_image.h>

#include <velox/velox.h>
#include <velox/module.h>
#include <velox/hook.h>
#include <velox/ewmh.h>
#include <velox/debug.h>

#include <velox/x11/event_handler.h>

/* Icons */
#include "bar_icons/clock.xbm"

const char name[] = "bar";

enum align
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

struct pixmap
{
    xcb_pixmap_t pixmap;
    uint32_t width;
};

static void workspace_list_item(struct pixmap * pixmap);

static xcb_window_t window;
static xcb_font_t font;

typedef void (* item_t)(struct pixmap * pixmap);

struct velox_vector items[3];

static uint32_t default_background_pixel;
static uint32_t default_foreground_pixel;
static uint32_t selected_background_pixel;
static uint32_t selected_foreground_pixel;
static xcb_gcontext_t default_background_gc;
static xcb_gcontext_t default_foreground_gc;
static xcb_gcontext_t selected_background_gc;
static xcb_gcontext_t selected_foreground_gc;

/* Configuration parameters */
static int bar_height = 14;
static int spacing = 12;
static const char * const font_name = "-*-terminus-medium-*-*-*-14-*-*-*-*-*-*-*";
static uint16_t default_background_color[] = { 0x1111, 0x1111, 0x1111 };
static uint16_t default_foreground_color[] = { 0x9999, 0x9999, 0x9999 };
static uint16_t selected_background_color[] = { 0x3333, 0x8888, 0x3333 };
static uint16_t selected_foreground_color[] = { 0xFFFF, 0xFFFF, 0xFFFF };

/* Items */
static void divider_item(struct pixmap * pixmap)
{
    pixmap->pixmap = xcb_generate_id(c);
    pixmap->width = spacing + 2;

    xcb_create_pixmap(c, screen->root_depth, pixmap->pixmap, window, pixmap->width, bar_height);

    xcb_poly_fill_rectangle(c, pixmap->pixmap, default_background_gc, 1, (xcb_rectangle_t[])
        { 0, 0, pixmap->width, bar_height });
    xcb_poly_fill_rectangle(c, pixmap->pixmap, default_foreground_gc, 1, (xcb_rectangle_t[])
        { spacing / 2, 0, 2, bar_height });
}

static void text_item(struct pixmap * pixmap, const char * const text)
{
    uint32_t index;
    uint32_t length = strlen(text);
    xcb_char2b_t text2b[length];

    xcb_query_text_extents_cookie_t text_extents_cookie;
    xcb_query_text_extents_reply_t * text_extents_reply;

    pixmap->pixmap = xcb_generate_id(c);

    for (index = 0; index < length; ++index)
    {
        text2b[index] = (xcb_char2b_t) { 0, text[index] };
    }

    text_extents_cookie = xcb_query_text_extents(c, font, length, text2b);
    text_extents_reply = xcb_query_text_extents_reply(c, text_extents_cookie, NULL);

    pixmap->width = text_extents_reply->overall_width + spacing;

    xcb_create_pixmap(c, screen->root_depth, pixmap->pixmap, window, pixmap->width, bar_height);

    xcb_poly_fill_rectangle(c, pixmap->pixmap, default_background_gc, 1, (xcb_rectangle_t[])
        { 0, 0, pixmap->width, bar_height });

    xcb_image_text_8(c, length, pixmap->pixmap, default_foreground_gc, spacing / 2,
        (bar_height + text_extents_reply->font_ascent - text_extents_reply->font_descent) / 2,
        text);

    free(text_extents_reply);
}

static void bitmap_item(struct pixmap * pixmap, uint32_t width, uint32_t height,
    uint32_t bits_length, uint8_t * bits)
{
    xcb_gcontext_t gc;

    pixmap->pixmap = xcb_create_pixmap_from_bitmap_data(c, window, bits, width, height, screen->root_depth,
        selected_background_pixel, default_background_pixel, NULL);
    pixmap->width = width;
}

static void clock_item(struct pixmap * pixmap)
{
    time_t raw_time = time(NULL);
    struct tm * local_time = localtime(&raw_time);
    char time_string[256];

    strftime(time_string, 256, "%A %T %F", local_time);
    text_item(pixmap, time_string);
}

static void clock_icon_item(struct pixmap * pixmap)
{
    bitmap_item(pixmap, clock_width, clock_height, sizeof(clock_bits), clock_bits);
}

static void window_title_item(struct pixmap * pixmap)
{
    struct velox_window * window = NULL;

    if (workspace->focus_type == TILE && !list_is_empty(&workspace->tiled.windows))
    {
        window = link_entry(workspace->tiled.focus, struct velox_window);
    }
    else if (!list_is_empty(&workspace->floated.windows))
    {
        window = list_first(&workspace->floated.windows, struct velox_window);
    }

    if (window)
    {
        text_item(pixmap, window->name);
    }
}

static void workspace_list_item(struct pixmap * pixmap)
{
    struct velox_workspace * workspace_iterator;
    uint32_t index;
    uint32_t length;
    uint32_t x = 0;
    xcb_char2b_t text2b[256];
    xcb_query_text_extents_cookie_t text_extents_cookies[workspaces.size];
    xcb_query_text_extents_reply_t * text_extents_replies[workspaces.size];
    uint32_t widths[workspaces.size];

    pixmap->pixmap = xcb_generate_id(c);
    pixmap->width = 0;

    vector_for_each(&workspaces, workspace_iterator)
    {
        for (index = 0, length = strlen(workspace_iterator->name); index < length; ++index)
        {
            text2b[index] = (xcb_char2b_t) { 0, workspace_iterator->name[index] };
        }

        text_extents_cookies[vector_position(&workspaces, workspace_iterator)] =
            xcb_query_text_extents(c, font, length, text2b);
    }

    for (index = 0; index < workspaces.size; ++index)
    {
        text_extents_replies[index] = xcb_query_text_extents_reply(c,
            text_extents_cookies[index], NULL);

        widths[index] = text_extents_replies[index]->overall_width + spacing;
        pixmap->width += widths[index];
    }

    xcb_create_pixmap(c, screen->root_depth, pixmap->pixmap, window, pixmap->width, bar_height);

    xcb_poly_fill_rectangle(c, pixmap->pixmap, default_background_gc, 1, (xcb_rectangle_t[])
        { 0, 0, pixmap->width, bar_height });

    vector_for_each_with_index(&workspaces, workspace_iterator, index)
    {
        if (workspace_iterator == workspace)
        {
            xcb_poly_fill_rectangle(c, pixmap->pixmap, selected_background_gc, 1,
                (xcb_rectangle_t[]) { x, 0, widths[index], bar_height });
        }

        xcb_image_text_8(c, strlen(workspace_iterator->name), pixmap->pixmap,
            workspace_iterator == workspace ? selected_foreground_gc : default_foreground_gc,
            x + spacing / 2, (bar_height + text_extents_replies[index]->font_ascent -
            text_extents_replies[index]->font_descent) / 2, workspace_iterator->name);

        x += widths[index];
    }
}

static void redraw()
{
    uint32_t mask;
    uint32_t values[3];
    uint32_t index;
    uint32_t alignment;
    uint32_t start_x[3];
    struct pixmap align_left_pixmaps[items[ALIGN_LEFT].size];
    struct pixmap align_center_pixmaps[items[ALIGN_CENTER].size];
    struct pixmap align_right_pixmaps[items[ALIGN_RIGHT].size];
    struct pixmap * pixmaps[] = {
        align_left_pixmaps,
        align_center_pixmaps,
        align_right_pixmaps
    };
    uint32_t x;
    item_t * item_iterator;

    /* Calculate starting x values */
    start_x[ALIGN_LEFT] = 0;
    start_x[ALIGN_CENTER] = screen_area.width / 2;
    start_x[ALIGN_RIGHT] = screen_area.width;

    vector_for_each_with_index(&items[ALIGN_LEFT], item_iterator, index)
    {
        (*item_iterator)(&align_left_pixmaps[index]);
    }

    vector_for_each_with_index(&items[ALIGN_CENTER], item_iterator, index)
    {
        (*item_iterator)(&align_center_pixmaps[index]);
        start_x[ALIGN_CENTER] -= align_center_pixmaps[index].width / 2;
    }

    vector_for_each_with_index(&items[ALIGN_RIGHT], item_iterator, index)
    {
        (*item_iterator)(&align_right_pixmaps[index]);
        start_x[ALIGN_RIGHT] -= align_right_pixmaps[index].width;
    }

    /* Draw items */
    xcb_poly_fill_rectangle(c, window, default_background_gc, 1, (xcb_rectangle_t[])
        { 0, 0, screen_area.width, bar_height });

    for (alignment = ALIGN_LEFT; alignment <= ALIGN_RIGHT; ++alignment)
    {
        x = start_x[alignment];

        for (index = 0; index < items[alignment].size; ++index)
        {
            xcb_copy_area(c, pixmaps[alignment][index].pixmap, window, default_foreground_gc,
                0, 0, x, 0, pixmaps[alignment][index].width, bar_height);
            xcb_free_pixmap(c, pixmaps[alignment][index].pixmap);
            x += pixmaps[alignment][index].width;
        }
    }

    xcb_flush(c);
};

static void redraw_hook(union velox_argument argument)
{
    redraw();
}

static void resize_hook(union velox_argument argument)
{
    DEBUG_ENTER

    xcb_configure_window(c, window, XCB_CONFIG_WINDOW_WIDTH,
        (uint32_t[]) { screen_area.width });

    xcb_ewmh_set_wm_strut_partial(ewmh, window, (xcb_ewmh_wm_strut_partial_t) {
        .top = bar_height, .top_start_x = 0,
        .top_end_x = screen_area.width
    });
}

static void expose(xcb_expose_event_t * event)
{
    if (event->window != window || event->count != 0) return;

    redraw();
}

bool setup()
{
    xcb_alloc_color_cookie_t default_background_cookie;
    xcb_alloc_color_cookie_t default_foreground_cookie;
    xcb_alloc_color_cookie_t selected_background_cookie;
    xcb_alloc_color_cookie_t selected_foreground_cookie;
    xcb_alloc_color_reply_t * default_background_reply;
    xcb_alloc_color_reply_t * default_foreground_reply;
    xcb_alloc_color_reply_t * selected_background_reply;
    xcb_alloc_color_reply_t * selected_foreground_reply;
    uint32_t mask;
    uint32_t values[3];

    printf("Bar: Initializing...");

    /* Pixmap Vectors */
    vector_initialize(&items[ALIGN_LEFT], sizeof(item_t), 16);
    vector_initialize(&items[ALIGN_CENTER], sizeof(item_t), 16);
    vector_initialize(&items[ALIGN_RIGHT], sizeof(item_t), 16);

    /* Allocate colors */
    default_background_cookie = xcb_alloc_color(c, screen->default_colormap,
        default_background_color[0],
        default_background_color[1],
        default_background_color[2]);
    default_foreground_cookie = xcb_alloc_color(c, screen->default_colormap,
        default_foreground_color[0],
        default_foreground_color[1],
        default_foreground_color[2]);
    selected_background_cookie = xcb_alloc_color(c, screen->default_colormap,
        selected_background_color[0],
        selected_background_color[1],
        selected_background_color[2]);
    selected_foreground_cookie = xcb_alloc_color(c, screen->default_colormap,
        selected_foreground_color[0],
        selected_foreground_color[1],
        selected_foreground_color[2]);

    default_background_reply = xcb_alloc_color_reply(c, default_background_cookie, NULL);
    default_foreground_reply = xcb_alloc_color_reply(c, default_foreground_cookie, NULL);
    selected_background_reply = xcb_alloc_color_reply(c, selected_background_cookie, NULL);
    selected_foreground_reply = xcb_alloc_color_reply(c, selected_foreground_cookie, NULL);

    default_background_pixel = default_background_reply->pixel;
    default_foreground_pixel = default_foreground_reply->pixel;
    selected_background_pixel = selected_background_reply->pixel;
    selected_foreground_pixel = selected_foreground_reply->pixel;

    /* Font */
    font = xcb_generate_id(c);
    xcb_open_font(c, font, strlen(font_name), font_name);

    /* Create the window */
    window = xcb_generate_id(c);

    mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    values[0] = default_background_pixel;
    values[1] = true;
    values[2] = XCB_EVENT_MASK_EXPOSURE;

    xcb_create_window(c, XCB_COPY_FROM_PARENT, window, screen->root,
        0, 0, screen->width_in_pixels, bar_height, 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

    /* Graphics Contexts */
    default_foreground_gc = xcb_generate_id(c);
    default_background_gc = xcb_generate_id(c);
    selected_foreground_gc = xcb_generate_id(c);
    selected_background_gc = xcb_generate_id(c);

    mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;

    values[0] = default_foreground_pixel;
    values[1] = default_background_pixel;
    values[2] = font;

    xcb_create_gc(c, default_foreground_gc, window, mask, values);

    values[0] = default_background_pixel;
    values[1] = default_foreground_pixel;

    xcb_create_gc(c, default_background_gc, window, mask, values);

    values[0] = selected_foreground_pixel;
    values[1] = selected_background_pixel;

    xcb_create_gc(c, selected_foreground_gc, window, mask, values);

    values[0] = selected_background_pixel;
    values[1] = selected_foreground_pixel;

    xcb_create_gc(c, selected_background_gc, window, mask, values);

    /* Struts */
    xcb_ewmh_set_wm_strut_partial(ewmh, window, (xcb_ewmh_wm_strut_partial_t) {
        .top = bar_height, .top_start_x = 0,
        .top_end_x = screen_area.width
    });

    xcb_map_window(c, window);

    xcb_flush(c);

    /* Register hooks and event handler */
    add_hook(&redraw_hook, VELOX_HOOK_TAG_CHANGED);
    add_hook(&redraw_hook, VELOX_HOOK_CLOCK_TICK);
    add_hook(&redraw_hook, VELOX_HOOK_FOCUS_CHANGED);
    add_hook(&redraw_hook, VELOX_HOOK_WINDOW_NAME_CHANGED);
    add_hook(&resize_hook, VELOX_HOOK_ROOT_RESIZED);
    add_expose_event_handler(&expose);

    /* Add bar items */
    vector_add_value(&items[ALIGN_LEFT], &workspace_list_item);
    vector_add_value(&items[ALIGN_LEFT], &divider_item);
    vector_add_value(&items[ALIGN_LEFT], &window_title_item);
    vector_add_value(&items[ALIGN_RIGHT], &clock_icon_item);
    vector_add_value(&items[ALIGN_RIGHT], &clock_item);

    printf("done\n");

    return true;
}

void cleanup()
{
    printf("Bar: Cleaning up...");

    xcb_close_font(c, font);
    xcb_destroy_window(c, window);
    xcb_free_gc(c, default_foreground_gc);
    xcb_free_gc(c, default_background_gc);
    xcb_free_gc(c, selected_foreground_gc);
    xcb_free_gc(c, selected_background_gc);

    vector_free(&items[ALIGN_LEFT]);
    vector_free(&items[ALIGN_CENTER]);
    vector_free(&items[ALIGN_RIGHT]);

    printf("done\n");
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

