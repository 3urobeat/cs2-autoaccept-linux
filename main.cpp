/*
 * File: main.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2021-06-04 17:00:05
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-18 10:44:28
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


    // TODO: Investigate: https://github.com/python-pillow/Pillow/issues/6392


    setup_portal();
    take_screenshot();
    destroy_portal();


    /* get_mouse();
    sleep(1);

    set_mouse_pos(50, 200);
    usleep(500000);
    mouse_click(1);

    sleep(1);

    signal_handler(0); */

    // Run intervalEvent() every checkInterval ms
    /* while (true) {
        intervalEvent();

        i++; // Increase counter

        auto x = chrono::steady_clock::now() + chrono::milliseconds(INTERVAL);
        this_thread::sleep_until(x);
    } */

    return 0;
}
