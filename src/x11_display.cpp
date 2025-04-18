/*
 * File: x11_display.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 15:07:27
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-18 15:54:39
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


Display *display;
Window   root;


XImage *x11_take_screenshot(int width, int height)
{
    return XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
}


void x11_get_display(int *width, int *height)
{
    // Establish connection to the X11 server: https://stackoverflow.com/questions/24988164/c-fast-screenshots-in-linux-for-use-with-opencv & https://stackoverflow.com/questions/4049877/how-to-save-ximage-as-bitmap
    display    = XOpenDisplay(0);
    int screen = XDefaultScreen(display);
    root       = RootWindow(display, screen);

    *width  = XDisplayWidth(display, screen); // This seems to return both monitors combined. If this impacts the scanning speed severely this needs to be fixed (seems to be fine)
    *height = XDisplayHeight(display, screen);
}
