/*
 * File: wayland_display.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-18 13:39:31
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-18 17:43:36
 * Modified By: 3urobeat
 *
 * Copyright (c) 2025 3urobeat <https://github.com/3urobeat>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "main.h"

XdpPortal *portal;
GMainLoop *loop = g_main_loop_new(NULL, FALSE);


// Callback when screenshot is done
void on_screenshot_response(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    char *response = xdp_portal_take_screenshot_finish(portal, res, &error);

    if (error) {
        cerr << "\nScreenshot failed: " << error->message << endl;
        g_error_free(error);
        return;
    }

    //cout << "\nScreenshot saved at: " << response << endl;
    g_main_loop_quit(loop);
}


void wl_take_screenshot(void (*screenshot_callback)(XImage *img)) // Yeah, I know, using XImage for Wayland ehhhhhh
{
    // Create a new portal and request a screenshot
    portal = xdp_portal_new();

    xdp_portal_take_screenshot(
        portal,
        NULL,
        XDP_SCREENSHOT_FLAG_NONE,
        NULL,
        on_screenshot_response,
        NULL
    );

    // Start a loop to prevent the application exiting before callback is called
    g_main_loop_run(loop);
}
