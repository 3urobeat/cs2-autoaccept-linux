/*
 * File: process_image.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 15:21:08
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-20 11:37:07
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


// IN:  *img OR (*png AND *info)
// OUT: *match_x, *match_y
bool process_image(XImage *img, png_structp *png, png_infop *info, int *match_x, int *match_y)
{
    bool breakLoop = false;
    int  matches   = 0;

    int width;
    int height;
    png_bytep* row_pointers;

    if (img)
    {
        width = img->width;
        height = img->height;
    }
    else
    {
        // Load the image we received into memory
        width = png_get_image_width(*png, *info);
        height = png_get_image_height(*png, *info);

        row_pointers = new png_bytep[height];
        for (int y = 0; y < height; y++)
        {
            row_pointers[y] = new png_byte[png_get_rowbytes(*png, *info)];
        }

        png_read_image(*png, row_pointers);
    }


    // Iterate over every pixel in the screenshot
    for (int row = 0; row < width && !breakLoop; row++) // x axis
    {
        for (int col = 0; col < height && !breakLoop; col++) // y axis
        {
            // Get the color of this pixel
            unsigned short red;
            unsigned short green;
            unsigned short blue;

            if (img)
            {
                unsigned long pxl = XGetPixel(img, row, col);
                red   = (pxl >> 16) & 0xff;
                green = (pxl >>  8) & 0xff;
                blue  = (pxl >>  0) & 0xff;
            }
            else
            {
                png_bytep pxl = &(row_pointers[col][row * 4]);
                red   = pxl[0];
                green = pxl[1];
                blue  = pxl[2];
            }

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
                *match_x = row;
                *match_y = col;

                breakLoop = true;
                break;
            }
        }
    }
    //cout << "\nMatches: " << matches << endl;


    // Cleanup
    if (png)
    {
        for (int y = 0; y < height; y++) {
            delete[] row_pointers[y];
        }
        delete[] row_pointers;
    }


    // If loop was aborted a match was found
    if (breakLoop)
    {
        return true;
    }
    else
    {
        return false;
    }
}
