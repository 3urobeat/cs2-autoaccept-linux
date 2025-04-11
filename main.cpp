/*
 * File: main.cpp
 * Project: csgo-autoaccept-cpp
 * Created Date: 2021-06-04 17:00:05
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-11 14:37:59
 * Modified By: 3urobeat
 *
 * Copyright (c) 2021 - 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <chrono>
#include <thread>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h> // Used to simulate mouse click

#define VERSION "2.0"
#define INTERVAL 4000   // Time in ms to wait between searches

using namespace std;


// Reusable variables
int      screen, i, x, y, width, height;
Display *display;
Window   root;
XImage  *img;


// Function that will get executed every checkInterval ms to check the screen for the 'Accept' button
void intervalEvent()
{
    cout << "\r[" << i << "] Searching..." << flush; // Print and let it replace itself

    //auto startTime = chrono::steady_clock::now(); // Only needed for testing to measure time this interval takes
    bool breakLoop = false;
    int  matches   = 0;

    // Make a screenshot
    img = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);

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
            if (red == 0
                && green >= 60 && green <= 210  // TODO: Test different brightness settings
                && blue >= 10 && blue <= 90)
            {
                //cout << "Match at " << row << "x" << col << "!" << endl;
                matches++;
            }

            // If we got 9000 matching pixels then it surely is the Accept button
            if (matches >= 9000)
            {
                cout << "\r--------------------------------------------" << endl;
                cout << "[" << i << "] Button found! Accepting match..." << endl;
                cout << "\nPlease close this window if everyone accepted, I will otherwise continue searching.\n" << endl;

                // Set cursor position, click and release (https://www.linuxquestions.org/questions/programming-9/simulating-a-mouse-click-594576/#post2936738)
                XWarpPointer(display, None, root, 0, 0, 0, 0, row, col); // Update cursor position
                XTestFakeButtonEvent(display, 1, True, CurrentTime);     // Press and release left click
                XTestFakeButtonEvent(display, 1, False, CurrentTime);
                XFlush(display);                                         // Necessary to execute XWarpPointer

                // Stop loop prematurely
                breakLoop = true;
                break;
            }
        }
    }

    // Release memory used by screenshot to avoid creating a leak
    XDestroyImage(img);

    //auto endTime = chrono::steady_clock::now();
    //cout << "\nMatches: " << matches << endl;
    //cout << "This iteration took " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << "ms.\n" << endl;
}


int main() // Entry point
{
    cout << "\ncs2-autoaccept-linux v" << VERSION << " by 3urobeat" << endl;
    cout << "--------------------------------------------" << endl;
    cout << "\nSearching screen for 'Accept' window every " << INTERVAL / 1000 << " seconds..." << endl;


    // Establish connection to the X11 server https://stackoverflow.com/questions/24988164/c-fast-screenshots-in-linux-for-use-with-opencv & https://stackoverflow.com/questions/4049877/how-to-save-ximage-as-bitmap
    display = XOpenDisplay(0);
    screen  = XDefaultScreen(display);
    root    = RootWindow(display, screen);

    x = 0, y = 0;
    width  = XDisplayWidth(display, screen); // This seems to return both monitors combined. If this impacts the scanning speed severely this needs to be fixed (seems to be fine)
    height = XDisplayHeight(display, screen);


    // Run intervalEvent() every checkInterval ms
    while (true) {
        intervalEvent();

        i++; // Increase counter

        auto x = chrono::steady_clock::now() + chrono::milliseconds(INTERVAL);
        this_thread::sleep_until(x);
    }
}
