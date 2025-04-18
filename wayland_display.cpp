#include "main.h"

XdpPortal *portal;
GMainLoop *loop = g_main_loop_new(NULL, FALSE);


// Callback when screenshot is done
void on_screenshot_response(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    char *response = xdp_portal_take_screenshot_finish(portal, res, &error);

    if (error) {
        cerr << "Screenshot failed: " << error->message << endl;
        g_error_free(error);
        return;
    }

    cout << "Screenshot saved at: " << response << endl;
    g_main_loop_quit(loop);
}


void take_screenshot()
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
