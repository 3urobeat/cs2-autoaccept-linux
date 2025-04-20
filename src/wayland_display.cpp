/*
 * File: wayland_display.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 13:39:31
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-20 11:37:21
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


XdpPortal *portal = xdp_portal_new(); // TODO: Will this cause issues on X11 systems without xdg-portals?
GMainLoop *loop = g_main_loop_new(NULL, FALSE);

void (*ss_callback_func)(XImage *img, png_structp *png, png_infop *info) = nullptr;


// Helper function that loads the png file into a libpng struct
void load_png(const char* filename)
{
    // Load file from disk
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        cerr << "Error: File could not be opened for reading: " << filename << endl;
        return;
    }

    // Create a libpng struct for image content and image metadata
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        cerr << "Error: Failed to create png_structp!" << endl;
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        cerr << "Error: Failed to create png_infop!" << endl;
        png_destroy_read_struct(&png, nullptr, nullptr);
        fclose(fp);
        return;
    }

    // Setup error handling to clean up
    if (setjmp(png_jmpbuf(png))) {
        cerr << "Error: setjmp failed" << endl;
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
        return;
    }


    // Load image metadata into libpng (not the actual image data yet! This will be done by process_image)
    png_init_io(png, fp);
    png_read_info(png, info);

    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    // Do some color handling which I don't actually fully understand, sorry
    png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png);
    }
    if (bit_depth == 16)
    {
        png_set_strip_16(png);
    }
    if (bit_depth < 8)
    {
        png_set_expand(png);
    }

    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    png_read_update_info(png, info);


    // Make callback
    ss_callback_func(nullptr, &png, &info);

    // Cleanup
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);
}


// Callback when screenshot is done
void on_screenshot_response(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    // Get filepath of the screenshot
    GError *error = NULL;
    char *filepath = xdp_portal_take_screenshot_finish(portal, res, &error);

    if (error) {
        cerr << "\nScreenshot failed: " << error->message << endl;
        g_error_free(error);
        return;
    }

    // Check if string starts with file:// and remove that
    if (strncmp(filepath, "file://", strlen("file://")) == 0)
    {
        filepath = filepath + 7; // Move pointer to after "file://"
    }

    //cout << "\nScreenshot saved at: " << filepath << endl;
    g_main_loop_quit(loop);

    // Parse file into XImage
    load_png(filepath);

    // Delete png file to not clutter up user's hard drive
    if (remove(filepath) != 0) {
        perror("Error: Failed to delete temporary screenshot file");
    }
}


void wl_take_screenshot(void (*screenshot_callback)(XImage *img, png_structp *png, png_infop *info))
{
    // Save reference to callback function
    ss_callback_func = screenshot_callback;

    // Request a screenshot
    xdp_portal_take_screenshot(
        portal,
        NULL,
        XDP_SCREENSHOT_FLAG_NONE,
        NULL,
        on_screenshot_response,
        NULL
    );

    // Start a loop to prevent the application exiting before callback is called
    g_main_loop_run(loop);
}
