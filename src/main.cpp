/*
 * File: main.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2021-06-04 17:00:05
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-19 13:18:01
 * Modified By: 3urobeat
 *
 * Copyright (c) 2021 - 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


int x11_width = 0;
int x11_height = 0;
int i = 0;
bool isUsingWayland;

// TODO: Catch exit and clean up screenshot

// Wayland screenshots are async as they might wait for user approval for example, so we provide a callback function to process the screenshot taken
void screenshot_callback(png_structp *png, png_infop *info)
{
    //auto startTime = chrono::steady_clock::now(); // Only needed for testing to measure time this interval takes // TODO: Doesn't factor in screenshot taking atm

    // Sanity check
    if (!png)
    {
        cout << "\nError: Did not receive a screenshot in callback! Did the screenshot fail?" << endl;
        return;
    }

    // Process screenshot
    int match_x;
    int match_y;
    bool match = process_image(png, info, &match_x, &match_y);

    // Print success message and manipulate cursor if a match was found
    if (match)
    {
        cout << "\r\x1b[32m[" << i << "] Button found! Accepting match...\x1b[0m" << endl;
        cout << "\nPlease close this window if everyone accepted and you are in the loading screen.\nI will otherwise continue searching.\n" << endl;

        // Set cursor position, click and release
        if (isUsingWayland)
        {
            wl_set_mouse_pos(match_x, match_y);
            usleep(100000); // 100ms
            wl_mouse_click(1);
            usleep(100000); // 100ms
            wl_mouse_click(0);
        }
        else
        {
            x11_set_mouse_pos(match_x, match_y);
            usleep(100000); // 100ms
            x11_mouse_click(1);
            usleep(100000); // 100ms
            x11_mouse_click(0);
        }
    }

    //auto endTime = chrono::steady_clock::now();
    //cout << "This iteration took " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << "ms.\n" << endl;
}


// Function that will get executed every checkInterval ms to check the screen for the 'Accept' button
void intervalEvent()
{
    cout << "\r[" << i << "] Searching..." << flush; // Print and let it replace itself

    // Take screenshot
    if (isUsingWayland)
    {
        wl_take_screenshot(&screenshot_callback);
    }
    else
    {
        x11_take_screenshot(x11_width, x11_height, &screenshot_callback);
    }
}


// Entry point
int main(int argc, char *argv[])
{
    // Set terminal title
    cout << "\033]0;cs2-autoaccept-linux v" << VERSION << " by 3urobeat\007";

    // Print welcome message
    cout << "\n\x1b[36m            cs2-autoaccept-linux v" << VERSION << " by 3urobeat\x1b[0m" << endl; // Cyan and Reset color codes at the beginning and end of the string
    cout << "---------------------------------------------------------------"  << endl;

    // Determine display server
    if (getenv("WAYLAND_DISPLAY"))
    {
        cout << "Detected Wayland environment..." << endl;
        isUsingWayland = true;
    }
    else if (getenv("DISPLAY"))
    {
        cout << "Detected X11 environment..." << endl;
        isUsingWayland = false;
    }
    else
    {
        die("Fatal: No display server detected! Are you running either X11 or Wayland?");
    }

    // Setup screen
    if (!isUsingWayland)
    {
        x11_get_display(&x11_width, &x11_height);
    }


    // Wayland test area
    /* wl_take_screenshot();
    sleep(1);
    wl_take_screenshot();


    wl_get_mouse();
    sleep(1);

    wl_set_mouse_pos(50, 200);
    usleep(500000);
    wl_mouse_click(1);

    sleep(1);

    wl_mouse_cleanup(0); */


    // Run intervalEvent() every checkInterval ms
    cout << "Checking your screen for a 'Accept' window every " << INTERVAL / 1000 << " second(s)...\n" << endl;

    while (true) {
        intervalEvent();

        i++; // Increase counter

        auto x = chrono::steady_clock::now() + chrono::milliseconds(INTERVAL);
        this_thread::sleep_until(x);
    }

    return 0;
}
