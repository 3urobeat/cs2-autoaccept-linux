/*
 * File: main.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2021-06-04 17:00:05
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-16 21:45:26
 * Modified By: 3urobeat
 *
 * Copyright (c) 2021 - 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


// Reusable variables
int      screen, i, x, y, width, height;

int write_to_ppm_stream(pixman_image_t *image, FILE *stream) {
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


void wayland()
{
    double scale = 1.0;
    bool use_greatest_scale = true;
    struct grim_box *geometry = NULL;
    char *geometry_output = NULL;
    int jpeg_quality = 80;
    int png_level = 6; // current default png/zlib compression level
    bool with_cursor = false;
    int opt;
    char tmp[64];


    struct grim_state state = {0};
    wl_list_init(&state.outputs);

    state.display = wl_display_connect(NULL);
    if (state.display == NULL) {
        fprintf(stderr, "failed to create display\n");
        return;
    }

    state.registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(state.registry, &registry_listener, &state);
    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr, "wl_display_roundtrip() failed\n");
        return;
    }

    if (state.shm == NULL) {
        fprintf(stderr, "compositor doesn't support wl_shm\n");
        return;
    }
    if ((state.ext_output_image_capture_source_manager == NULL || state.ext_image_copy_capture_manager == NULL) &&
            state.screencopy_manager == NULL) {
        fprintf(stderr, "compositor doesn't support the screencopy protocol\n");
        return;
    }
    if (wl_list_empty(&state.outputs)) {
        fprintf(stderr, "no wl_output\n");
        return;
    }

    if (state.xdg_output_manager != NULL) {
        struct grim_output *output;
        wl_list_for_each(output, &state.outputs, link) {
            output->xdg_output = zxdg_output_manager_v1_get_xdg_output(
                state.xdg_output_manager, output->wl_output);
            zxdg_output_v1_add_listener(output->xdg_output,
                &xdg_output_listener, output);
        }

        if (wl_display_roundtrip(state.display) < 0) {
            fprintf(stderr, "wl_display_roundtrip() failed\n");
            return;
        }
    } else {
        fprintf(stderr, "warning: zxdg_output_manager_v1 isn't available, "
            "guessing the output layout\n");

        struct grim_output *output;
        wl_list_for_each(output, &state.outputs, link) {
            guess_output_logical_geometry(output);
        }
    }

    size_t n_pending = 0;
    struct grim_output *output;
    wl_list_for_each(output, &state.outputs, link) {
        if (geometry != NULL && !intersect_box(geometry, &output->logical_geometry)) {
            continue;
        }
        if (use_greatest_scale && output->logical_scale > scale) {
            scale = output->logical_scale;
        }

        if (state.ext_output_image_capture_source_manager != NULL) {
            uint32_t options = 0;

            struct ext_image_capture_source_v1 *source = ext_output_image_capture_source_manager_v1_create_source(
                state.ext_output_image_capture_source_manager, output->wl_output);
            output->ext_image_copy_capture_session = ext_image_copy_capture_manager_v1_create_session(
                state.ext_image_copy_capture_manager, source, options);
            ext_image_copy_capture_session_v1_add_listener(output->ext_image_copy_capture_session,
                &ext_image_copy_capture_session_listener, output);
            ext_image_capture_source_v1_destroy(source);
        } else {
            output->screencopy_frame = zwlr_screencopy_manager_v1_capture_output(
                state.screencopy_manager, with_cursor, output->wl_output);
            zwlr_screencopy_frame_v1_add_listener(output->screencopy_frame,
                &screencopy_frame_listener, output);
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
    FILE *file = fopen("/home/tomke/test", "w");
    int ret = write_to_ppm_stream(image, file);
    fclose(file);

    printf("Done!\n");


    // Clean up
    pixman_image_unref(image);

    struct grim_output *output_tmp;
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
    free(geometry_output);
}


// Function that will get executed every checkInterval ms to check the screen for the 'Accept' button
void intervalEvent()
{
    cout << "\r[" << i << "] Searching..." << flush; // Print and let it replace itself

    //auto startTime = chrono::steady_clock::now(); // Only needed for testing to measure time this interval takes
    bool breakLoop = false;
    int  matches   = 0;

    // Make a screenshot


    // Iterate over every pixel in the screenshot
    for (int row = 0; row < width && !breakLoop; row++) // x axis
    {
        for (int col = 0; col < height && !breakLoop; col++) // y axis
        {
            // Get the color of this pixel
            unsigned long  pxl   = 0;
            unsigned short red   = (pxl >> 16) & 0xff;
            unsigned short green = (pxl >>  8) & 0xff;
            unsigned short blue  = (pxl >>  0) & 0xff;
            //cout << "Pixel (" << row << "x" << col << ") Color (RGB): " << red << " " << green << " " << blue << endl;

            // Check if color is interesting
            if (red >= 50 && red <= 60
                && green >= 178 && green <= 187
                && blue >= 77 && blue <= 87)
            {
                //cout << "Match at " << row << "x" << col << "!" << endl;
                matches++;
            }

            // If we got 9000 matching pixels then it surely is the Accept button
            if (matches >= 9000)
            {
                cout << "\r\x1b[32m[" << i << "] Button found! Accepting match...\x1b[0m" << endl;
                cout << "\nPlease close this window if everyone accepted and you are in the loading screen.\nI will otherwise continue searching.\n" << endl;

                // Set cursor position, click and release

                // Stop loop prematurely
                breakLoop = true;
                break;
            }
        }
    }

    // Release memory used by screenshot to avoid creating a leak


    //auto endTime = chrono::steady_clock::now();
    //cout << "\nMatches: " << matches << endl;
    //cout << "This iteration took " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << "ms.\n" << endl;
}


int main(int argc, char *argv[]) // Entry point
{
    // Set terminal title
    cout << "\033]0;cs2-autoaccept-linux v" << VERSION << " by 3urobeat\007";

    // Print welcome message
    cout << "\n\x1b[36m            cs2-autoaccept-linux v" << VERSION << " by 3urobeat\x1b[0m" << endl; // Cyan and Reset color codes at the beginning and end of the string
    cout << "---------------------------------------------------------------"  << endl;
    cout << "Checking your screen for a 'Accept' window every " << INTERVAL / 1000 << " second(s)...\n" << endl;

    x = 0, y = 0;

    wayland();


    // Run intervalEvent() every checkInterval ms
    /* while (true) {
        intervalEvent();

        i++; // Increase counter

        auto x = chrono::steady_clock::now() + chrono::milliseconds(INTERVAL);
        this_thread::sleep_until(x);
    } */
}
