#include "main.h"

// Modified implementation of grim

struct grim_box *geometry = NULL;
char *geometry_output = NULL;
struct grim_state state = {0};


// For testing
int write_to_ppm_stream(pixman_image_t *image, FILE *stream)
{
	// 256 bytes ought to be enough for everyone
	char header[256];

	int width = pixman_image_get_width(image);
	int height = pixman_image_get_height(image);

	int header_len = snprintf(header, sizeof(header), "P6\n%d %d\n255\n", width, height);
	assert(header_len <= (int)sizeof(header));

	size_t len = header_len + width * height * 3;
	unsigned char *data = (unsigned char*) malloc(len);
	unsigned char *buffer = data;

	// We _do_not_ include the null byte
	memcpy(buffer, header, header_len);
	buffer += header_len;

	pixman_format_code_t format = pixman_image_get_format(image);
	assert(format == PIXMAN_a8r8g8b8 || format == PIXMAN_x8r8g8b8);

	// Both formats are native-endian 32-bit ints
	uint32_t *pixels = pixman_image_get_data(image);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uint32_t p = *pixels++;
			// RGB order
			*buffer++ = (p >> 16) & 0xff;
			*buffer++ = (p >>  8) & 0xff;
			*buffer++ = (p >>  0) & 0xff;
		}
	}

	size_t written = fwrite(data, 1, len, stream);
	if (written < len) {
		free(data);
		fprintf(stderr, "Failed to write ppm; only %zu of %zu bytes written\n",
			written, len);
		return -1;
	}
	free(data);
	return 0;
}


void get_display()
{
    wl_list_init(&state.outputs);

    state.display = wl_display_connect(NULL);
    if (state.display == NULL) {
        die("Error: Failed to create display");
    }

    state.registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(state.registry, &registry_listener, &state);
    if (wl_display_roundtrip(state.display) < 0) {
        die("wl_display_roundtrip() failed");
    }

    // Check compositor support
    if (state.shm == NULL) {
        die("Error: Compositor doesn't support wl_shm");
    }
    if ((state.ext_output_image_capture_source_manager == NULL || state.ext_image_copy_capture_manager == NULL) && state.screencopy_manager == NULL) {
        die("Error: Compositor doesn't support the screencopy protocol");
    }
    if (wl_list_empty(&state.outputs)) {
        die("Error: No wl_output");
    }

    if (state.xdg_output_manager != NULL) {
        struct grim_output *output;
        wl_list_for_each(output, &state.outputs, link) {
            output->xdg_output = zxdg_output_manager_v1_get_xdg_output(state.xdg_output_manager, output->wl_output);
            zxdg_output_v1_add_listener(output->xdg_output, &xdg_output_listener, output);
        }

        if (wl_display_roundtrip(state.display) < 0) {
            die("Error: wl_display_roundtrip() failed");
        }
    } else {
        fprintf(stderr, "warning: zxdg_output_manager_v1 isn't available, guessing the output layout\n");

        struct grim_output *output;
        wl_list_for_each(output, &state.outputs, link) {
            guess_output_logical_geometry(output);
        }
    }
}

void take_screenshot(const char *filename)
{
    struct grim_state oldstate;
    memcpy(&oldstate, &state, sizeof(state));


    double scale = 1.0;
    size_t n_pending = 0;
    struct grim_output *output;

    wl_list_for_each(output, &state.outputs, link) {
        if (geometry != NULL && !intersect_box(geometry, &output->logical_geometry)) {
            continue;
        }

        // Get display scale
        scale = output->logical_scale;

        // Use ext_image_copy or zlwr_screencopy to take screenshot
        if (state.ext_output_image_capture_source_manager != NULL) {
            uint32_t options = 0;

            struct ext_image_capture_source_v1 *source =
                ext_output_image_capture_source_manager_v1_create_source(state.ext_output_image_capture_source_manager, output->wl_output);

            output->ext_image_copy_capture_session =
                ext_image_copy_capture_manager_v1_create_session(state.ext_image_copy_capture_manager, source, options);

            ext_image_copy_capture_session_v1_add_listener(output->ext_image_copy_capture_session, &ext_image_copy_capture_session_listener, output);
            ext_image_capture_source_v1_destroy(source);
        } else {
            output->screencopy_frame = zwlr_screencopy_manager_v1_capture_output(state.screencopy_manager, false, output->wl_output);
            zwlr_screencopy_frame_v1_add_listener(output->screencopy_frame, &screencopy_frame_listener, output);
        }

        ++n_pending;
    }

    if (n_pending == 0) {
        fprintf(stderr, "supplied geometry did not intersect with any outputs\n");
        return;
    }

    bool done = false;
    while (!done && wl_display_dispatch(state.display) != -1) {
        done = (state.n_done == n_pending);
    }
    if (!done) {
        fprintf(stderr, "failed to screenshot all outputs\n");
        return;
    }

    if (geometry == NULL) {
        geometry = (grim_box*) calloc(1, sizeof(struct grim_box));
        get_output_layout_extents(&state, geometry);
    }

    pixman_image_t *image = render(&state, geometry, scale);
    if (image == NULL) {
        return;
    }

    // Save for testing
    FILE *file = fopen(filename, "w");
    int ret = write_to_ppm_stream(image, file);
    fclose(file);

    // Iterate over all pixels
    uint32_t* img_data = (uint32_t*)pixman_image_get_data(image);

    for (int row = 0; row < geometry->width; row++) {
        for (int col = 0; col < geometry->height; col++) {
            // Get the pixel value at (x, y)

            uint32_t pxl = img_data[col * geometry->width + row];

            // Extract RGB components
            unsigned short red   = (pxl >> 16) & 0xff;
            unsigned short green = (pxl >>  8) & 0xff;
            unsigned short blue  = (pxl >>  0) & 0xff;
            cout << "Pixel (" << row << "x" << col << ") Color (RGB): " << red << " " << green << " " << blue << endl;
        }
    }

    pixman_image_unref(image);
    printf("Done!\n");


    memcpy(&state, &oldstate, sizeof oldstate);
}

void clean_up()
{
    /* struct grim_output *output_tmp;
    wl_list_for_each_safe(output, output_tmp, &state.outputs, link) {
        wl_list_remove(&output->link);
        free(output->name);
        if (output->ext_image_copy_capture_frame != NULL) {
            ext_image_copy_capture_frame_v1_destroy(output->ext_image_copy_capture_frame);
        }
        if (output->ext_image_copy_capture_session != NULL) {
            ext_image_copy_capture_session_v1_destroy(output->ext_image_copy_capture_session);
        }
        if (output->screencopy_frame != NULL) {
            zwlr_screencopy_frame_v1_destroy(output->screencopy_frame);
        }
        destroy_buffer(output->buffer);
        if (output->xdg_output != NULL) {
            zxdg_output_v1_destroy(output->xdg_output);
        }
        wl_output_release(output->wl_output);
        free(output);
    }
    if (state.ext_output_image_capture_source_manager != NULL) {
        ext_output_image_capture_source_manager_v1_destroy(state.ext_output_image_capture_source_manager);
    }
    if (state.ext_image_copy_capture_manager != NULL) {
        ext_image_copy_capture_manager_v1_destroy(state.ext_image_copy_capture_manager);
    }
    if (state.screencopy_manager != NULL) {
        zwlr_screencopy_manager_v1_destroy(state.screencopy_manager);
    }
    if (state.xdg_output_manager != NULL) {
        zxdg_output_manager_v1_destroy(state.xdg_output_manager);
    }
    wl_shm_destroy(state.shm);
    wl_registry_destroy(state.registry);
    wl_display_disconnect(state.display);
    free(geometry);
    free(geometry_output); */
}
