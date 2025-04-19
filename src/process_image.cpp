/*
 * File: process_image.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 15:21:08
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-19 12:59:36
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


// IN:  *png, *info
// OUT: *match_x, *match_y
bool process_image(png_structp *png, png_infop *info, int *match_x, int *match_y)
{
    bool breakLoop = false;
    int  matches   = 0;


    // Load the image we received into memory
    int width = png_get_image_width(*png, *info);
    int height = png_get_image_height(*png, *info);

    png_bytep* row_pointers = new png_bytep[height];
    for (int y = 0; y < height; y++) {
        row_pointers[y] = new png_byte[png_get_rowbytes(*png, *info)];
    }

    png_read_image(*png, row_pointers);


    // Iterate over every pixel in the screenshot
    for (int row = 0; row < width && !breakLoop; row++) // x axis
    {
        for (int col = 0; col < height && !breakLoop; col++) // y axis
        {
            // Get the color of this pixel
            png_bytep pxl = &(row_pointers[col][row * 4]); // Assuming RGBA format
            unsigned short red   = pxl[0];
            unsigned short green = pxl[1];
            unsigned short blue  = pxl[2];
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
    for (int y = 0; y < height; y++) {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;


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
