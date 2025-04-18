/*
 * File: main.h
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-16 19:09:04
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-18 15:40:30
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#pragma once

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <libportal/portal.h> // Used to take screenshot on Wayland
//#include <pixman.h>
//#include <glib.h>
//#include <gio/gio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h> // Used to simulate mouse click when using X11

#define VERSION "1.2"
#define INTERVAL 4000   // Time in ms to wait between searches

using namespace std;


#define die(str, args...) do { \
    perror(str); \
    exit(EXIT_FAILURE); \
} while(0)



// process_image.cpp
extern bool process_image(XImage *img, int width, int height, int *match_x, int *match_y);

// wayland_display.cpp
extern void wl_take_screenshot();

// wayland_mouse.cpp
extern void wl_mouse_cleanup(int signo);
extern void wl_get_mouse();
extern void wl_set_mouse_pos(int x, int y);
extern void wl_mouse_click(int depressed);

// x11_display.cpp
extern Display *display;
extern Window   root;

extern XImage *x11_take_screenshot(int width, int height);
extern void    x11_get_display(int *width, int *height);

// x11_mouse.cpp
extern void x11_set_mouse_pos(int x, int y);
extern void x11_mouse_click(int depressed);
