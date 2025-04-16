#pragma once

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <pixman.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wordexp.h>

#include "buffer.h"
#include "grim.h"
#include "output-layout.h"
#include "render.h"
#if HAVE_JPEG
#include "write_jpg.h"
#endif

#include "ext-image-capture-source-v1-protocol.h"
#include "ext-image-copy-capture-v1-protocol.h"
#include "wlr-screencopy-unstable-v1-protocol.h"
#include "xdg-output-unstable-v1-protocol.h"


static void screencopy_frame_handle_buffer(void *data,
    struct zwlr_screencopy_frame_v1 *frame, uint32_t format, uint32_t width,
    uint32_t height, uint32_t stride) {
    struct grim_output *output = data;

    output->buffer =
        create_buffer(output->state->shm, format, width, height, stride);
    if (output->buffer == NULL) {
        fprintf(stderr, "failed to create buffer\n");
        exit(EXIT_FAILURE);
    }

    zwlr_screencopy_frame_v1_copy(frame, output->buffer->wl_buffer);
}

static void screencopy_frame_handle_flags(void *data,
    struct zwlr_screencopy_frame_v1 *frame, uint32_t flags) {
    struct grim_output *output = data;
    output->screencopy_frame_flags = flags;
}

static void screencopy_frame_handle_ready(void *data,
    struct zwlr_screencopy_frame_v1 *frame, uint32_t tv_sec_hi,
    uint32_t tv_sec_lo, uint32_t tv_nsec) {
    struct grim_output *output = data;
    ++output->state->n_done;
}

static void screencopy_frame_handle_failed(void *data,
    struct zwlr_screencopy_frame_v1 *frame) {
    struct grim_output *output = data;
    fprintf(stderr, "failed to copy output %s\n", output->name);
    exit(EXIT_FAILURE);
}

static const struct zwlr_screencopy_frame_v1_listener screencopy_frame_listener = {
    .buffer = screencopy_frame_handle_buffer,
    .flags = screencopy_frame_handle_flags,
    .ready = screencopy_frame_handle_ready,
    .failed = screencopy_frame_handle_failed,
};


static void ext_image_copy_capture_frame_handle_transform(void *data,
    struct ext_image_copy_capture_frame_v1 *frame, uint32_t transform) {
    struct grim_output *output = data;
    output->transform = transform;
}

static void ext_image_copy_capture_frame_handle_damage(void *data,
    struct ext_image_copy_capture_frame_v1 *frame, int32_t x, int32_t y,
    int32_t wdth, int32_t height) {
    // No-op
}

static void ext_image_copy_capture_frame_handle_presentation_time(void *data,
    struct ext_image_copy_capture_frame_v1 *frame, uint32_t tv_sec_hi,
    uint32_t tv_sec_lo, uint32_t tv_nsec) {
    // No-op
}

static void ext_image_copy_capture_frame_handle_ready(void *data,
    struct ext_image_copy_capture_frame_v1 *frame) {
    struct grim_output *output = data;
    ++output->state->n_done;
}

static void ext_image_copy_capture_frame_handle_failed(void *data,
    struct ext_image_copy_capture_frame_v1 *frame, uint32_t reason) {
    // TODO: retry depending on reason
    struct grim_output *output = data;
    fprintf(stderr, "failed to copy output %s\n", output->name);
    exit(EXIT_FAILURE);
}

static const struct ext_image_copy_capture_frame_v1_listener ext_image_copy_capture_frame_listener = {
    .transform = ext_image_copy_capture_frame_handle_transform,
    .damage = ext_image_copy_capture_frame_handle_damage,
    .presentation_time = ext_image_copy_capture_frame_handle_presentation_time,
    .ready = ext_image_copy_capture_frame_handle_ready,
    .failed = ext_image_copy_capture_frame_handle_failed,
};

static void ext_image_copy_capture_session_handle_buffer_size(void *data,
    struct ext_image_copy_capture_session_v1 *session, uint32_t width, uint32_t height) {
    struct grim_output *output = data;
    output->buffer_width = width;
    output->buffer_height = height;
}

static void ext_image_copy_capture_session_handle_shm_format(void *data,
    struct ext_image_copy_capture_session_v1 *session, uint32_t format) {
    struct grim_output *output = data;

    if (output->has_shm_format || !is_format_supported(format)) {
        return;
    }

    output->shm_format = format;
    output->has_shm_format = true;
}

static void ext_image_copy_capture_session_handle_dmabuf_device(void *data,
    struct ext_image_copy_capture_session_v1 *session, struct wl_array *dev_id_array) {
    // No-op
}

static void ext_image_copy_capture_session_handle_dmabuf_format(void *data,
    struct ext_image_copy_capture_session_v1 *session, uint32_t format,
    struct wl_array *modifiers_array) {
    // No-op
}

static void ext_image_copy_capture_session_handle_done(void *data,
    struct ext_image_copy_capture_session_v1 *session) {
    struct grim_output *output = data;

    if (output->ext_image_copy_capture_frame != NULL) {
        return;
    }

    if (!output->has_shm_format) {
        fprintf(stderr, "no supported format found\n");
        exit(EXIT_FAILURE);
    }

    int32_t stride = get_format_min_stride(output->shm_format, output->buffer_width);
    output->buffer =
        create_buffer(output->state->shm, output->shm_format, output->buffer_width, output->buffer_height, stride);
    if (output->buffer == NULL) {
        fprintf(stderr, "failed to create buffer\n");
        exit(EXIT_FAILURE);
    }

    output->ext_image_copy_capture_frame = ext_image_copy_capture_session_v1_create_frame(session);
    ext_image_copy_capture_frame_v1_add_listener(output->ext_image_copy_capture_frame,
        &ext_image_copy_capture_frame_listener, output);

    ext_image_copy_capture_frame_v1_attach_buffer(output->ext_image_copy_capture_frame, output->buffer->wl_buffer);
    ext_image_copy_capture_frame_v1_damage_buffer(output->ext_image_copy_capture_frame,
        0, 0, INT32_MAX, INT32_MAX);
    ext_image_copy_capture_frame_v1_capture(output->ext_image_copy_capture_frame);
}

static void ext_image_copy_capture_session_handle_stopped(void *data,
    struct ext_image_copy_capture_session_v1 *session) {
    // No-op
}

static const struct ext_image_copy_capture_session_v1_listener ext_image_copy_capture_session_listener = {
    .buffer_size = ext_image_copy_capture_session_handle_buffer_size,
    .shm_format = ext_image_copy_capture_session_handle_shm_format,
    .dmabuf_device = ext_image_copy_capture_session_handle_dmabuf_device,
    .dmabuf_format = ext_image_copy_capture_session_handle_dmabuf_format,
    .done = ext_image_copy_capture_session_handle_done,
    .stopped = ext_image_copy_capture_session_handle_stopped,
};


static void xdg_output_handle_logical_position(void *data,
    struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y) {
    struct grim_output *output = data;

    output->logical_geometry.x = x;
    output->logical_geometry.y = y;
}

static void xdg_output_handle_logical_size(void *data,
    struct zxdg_output_v1 *xdg_output, int32_t width, int32_t height) {
    struct grim_output *output = data;

    output->logical_geometry.width = width;
    output->logical_geometry.height = height;
}

static void xdg_output_handle_done(void *data,
    struct zxdg_output_v1 *xdg_output) {
    struct grim_output *output = data;

    // Guess the output scale from the logical size
    int32_t width = output->geometry.width;
    int32_t height = output->geometry.height;
    apply_output_transform(output->transform, &width, &height);
    output->logical_scale = (double)width / output->logical_geometry.width;
}

static void xdg_output_handle_name(void *data,
    struct zxdg_output_v1 *xdg_output, const char *name) {
    struct grim_output *output = data;
    if (output->name) {
        return; // prefer wl_output.name if available
    }
    output->name = strdup(name);
}

static void xdg_output_handle_description(void *data,
    struct zxdg_output_v1 *xdg_output, const char *name) {
    // No-op
}

static const struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_handle_logical_position,
    .logical_size = xdg_output_handle_logical_size,
    .done = xdg_output_handle_done,
    .name = xdg_output_handle_name,
    .description = xdg_output_handle_description,
};


static void output_handle_geometry(void *data, struct wl_output *wl_output,
    int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
    int32_t subpixel, const char *make, const char *model,
    int32_t transform) {
    struct grim_output *output = data;

    output->geometry.x = x;
    output->geometry.y = y;
    output->transform = transform;
}

static void output_handle_mode(void *data, struct wl_output *wl_output,
    uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
    struct grim_output *output = data;

    if ((flags & WL_OUTPUT_MODE_CURRENT) != 0) {
        output->geometry.width = width;
        output->geometry.height = height;
    }
}

static void output_handle_done(void *data, struct wl_output *wl_output) {
    // No-op
}

static void output_handle_scale(void *data, struct wl_output *wl_output,
    int32_t factor) {
    struct grim_output *output = data;
    output->scale = factor;
}

static void output_handle_name(void *data, struct wl_output *wl_output,
    const char *name) {
    struct grim_output *output = data;
    output->name = strdup(name);
}

static void output_handle_description(void *data, struct wl_output *wl_output,
    const char *description) {
    // No-op
}

static const struct wl_output_listener output_listener = {
    .geometry = output_handle_geometry,
    .mode = output_handle_mode,
    .done = output_handle_done,
    .scale = output_handle_scale,
    .name = output_handle_name,
    .description = output_handle_description,
};


static void handle_global(void *data, struct wl_registry *registry,
    uint32_t name, const char *interface, uint32_t version) {
    struct grim_state *state = data;

    if (strcmp(interface, wl_shm_interface.name) == 0) {
        state->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
        uint32_t bind_version = (version > 2) ? 2 : version;
        state->xdg_output_manager = wl_registry_bind(registry, name,
            &zxdg_output_manager_v1_interface, bind_version);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        uint32_t bind_version = (version >= 4) ? 4 : 3;
        struct grim_output *output = calloc(1, sizeof(struct grim_output));
        output->state = state;
        output->scale = 1;
        output->wl_output =  wl_registry_bind(registry, name,
            &wl_output_interface, bind_version);
        wl_output_add_listener(output->wl_output, &output_listener, output);
        wl_list_insert(&state->outputs, &output->link);
    } else if (strcmp(interface, ext_output_image_capture_source_manager_v1_interface.name) == 0) {
        state->ext_output_image_capture_source_manager = wl_registry_bind(registry, name,
            &ext_output_image_capture_source_manager_v1_interface, 1);
    } else if (strcmp(interface, ext_image_copy_capture_manager_v1_interface.name) == 0) {
        state->ext_image_copy_capture_manager = wl_registry_bind(registry, name,
            &ext_image_copy_capture_manager_v1_interface, 1);
    } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) == 0) {
        state->screencopy_manager = wl_registry_bind(registry, name,
            &zwlr_screencopy_manager_v1_interface, 1);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
    uint32_t name) {
    // who cares
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};
