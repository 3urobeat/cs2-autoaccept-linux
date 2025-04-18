/*
 * File: x11_mouse.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 15:10:05
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-18 15:22:06
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"


void x11_set_mouse_pos(int x, int y)
{
    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
    XFlush(display); // Necessary to execute call
}


void x11_mouse_click(int depressed)
{
    // Reference: https://www.linuxquestions.org/questions/programming-9/simulating-a-mouse-click-594576/#post2936738
    XTestFakeButtonEvent(display, 1, depressed, CurrentTime);
    XFlush(display); // Necessary to execute call
}
