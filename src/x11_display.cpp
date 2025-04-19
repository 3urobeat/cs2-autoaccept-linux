/*
 * File: x11_display.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 15:07:27
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-19 13:07:41
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include <cstddef>
#include <vector>

#include "main.h"


Display *display;
Window   root;


// Helper function that converts XImage into a libpng struct
void write_png(XImage *img, void (*screenshot_callback)(png_structp *png, png_infop *info))
{
    // Create a libpng struct for image content and image metadata
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
    {
        cerr << "Error: Failed to create png_structp!" << endl;
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        cerr << "Error: Failed to create png_infop!" << endl;
        png_destroy_write_struct(&png, nullptr);
        return;
    }

    // Setup error handling to clean up
    if (setjmp(png_jmpbuf(png)))
    {
        cerr << "Error: setjmp failed" << endl;
        png_destroy_read_struct(&png, &info, nullptr);
        return;
    }


    // Create a memory buffer to hold the PNG data
    vector<unsigned char> pngBuffer;
    size_t pngSize;

    pngBuffer.clear();
    pngBuffer.resize(img->width * img->height);

    // Setup writing image data
    //png_set_compression_level(png, 0);
    png_set_filter_heuristics(png, PNG_FILTER_HEURISTIC_UNWEIGHTED, 0, nullptr, nullptr);
    png_set_write_fn(png, &pngBuffer, [](png_structp png, png_bytep data, png_size_t length) {
        auto* buffer = static_cast<vector<unsigned char>*>(png_get_io_ptr(png));
        size_t currentSize = buffer->size();
        if (currentSize < length) {
            buffer->resize(currentSize + length);
        }
        copy(data, data + length, buffer->begin() + currentSize);
    }, nullptr);

    // Set header information
    png_set_IHDR(png, info, img->width, img->height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);


    // Write the image data
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img->height);
    for (int y = 0; y < img->height; y++)
    {
        row_pointers[y] = (png_bytep)(img->data + y * img->bytes_per_line);
    }

    png_write_image(png, row_pointers);
    png_write_end(png, nullptr);


    // Make callback
    screenshot_callback(&png, &info);

    // Clean up
    free(row_pointers);
    png_destroy_write_struct(&png, &info);
}


void x11_take_screenshot(int width, int height, void (*screenshot_callback)(png_structp *png, png_infop *info))
{
    // Take screenshot
    XImage *img = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);

    write_png(img, screenshot_callback);

    // Clean up
    XDestroyImage(img);
}


void x11_get_display(int *width, int *height)
{
    // Establish connection to the X11 server: https://stackoverflow.com/questions/24988164/c-fast-screenshots-in-linux-for-use-with-opencv & https://stackoverflow.com/questions/4049877/how-to-save-img-as-bitmap
    display    = XOpenDisplay(0);
    int screen = XDefaultScreen(display);
    root       = RootWindow(display, screen);

    *width  = XDisplayWidth(display, screen); // This seems to return both monitors combined. If this impacts the scanning speed severely this needs to be fixed (seems to be fine)
    *height = XDisplayHeight(display, screen);
}
