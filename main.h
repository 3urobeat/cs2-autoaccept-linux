/*
 * File: main.h
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-16 19:09:04
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-16 20:10:00
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#pragma once

#include <iostream>
#include <chrono>
#include <pixman.h>
#include <thread>
#include <wayland-client.h>

#include "include/grim/main.h"
#include "include/grim/include/box.h"
#include "include/grim/include/buffer.h"
#include "include/grim/include/output-layout.h"
#include "include/grim/include/render.h"
#include "include/grim/include/grim.h"

#include "include/grim/build/grim.p/ext-image-capture-source-v1-protocol.h"
#include "include/grim/build/grim.p/ext-image-copy-capture-v1-protocol.h"
#include "include/grim/build/grim.p/wlr-screencopy-unstable-v1-protocol.h"
#include "include/grim/build/grim.p/xdg-output-unstable-v1-protocol.h"


#define VERSION "1.2"
#define INTERVAL 4000   // Time in ms to wait between searches

using namespace std;

#define die(str, args...) do { \
    perror(str); \
    exit(EXIT_FAILURE); \
} while(0)

extern void get_display();
extern void take_screenshot(const char *filename);
extern void clean_up();

extern void signal_handler(int signo);
extern void get_mouse();
extern void set_mouse_pos(int x, int y);
extern void mouse_click(int depressed);
