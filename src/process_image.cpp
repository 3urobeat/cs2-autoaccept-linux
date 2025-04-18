/*
 * File: process_image.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 15:21:08
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-18 15:38:07
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


bool process_image(XImage *img, int width, int height, int *match_x, int *match_y)
{
    bool breakLoop = false;
    int  matches   = 0;

    // Iterate over every pixel in the screenshot
    for (int row = 0; row < width && !breakLoop; row++) // x axis
    {
        for (int col = 0; col < height && !breakLoop; col++) // y axis
        {
            // Get the color of this pixel
            unsigned long  pxl   = XGetPixel(img, row, col);
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
                *match_x = row;
                *match_y = col;

                breakLoop = true;
                return true;
                break;
            }
        }
    }

    return false;
}
