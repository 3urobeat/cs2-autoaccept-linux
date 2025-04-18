#include <cstdlib>
#include <iostream>
#include <gio/gio.h>
#include <glib.h>
#include <pixman.h>

#include "main.h"

GError *error;
GDBusProxy *proxy;


void setup_portal()
{
    // Initialize GIO
    //g_type_init();

    // Create a GError pointer to capture any errors
    error = nullptr;

    // Create a GDBusProxy for the screenshot portal
    proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION,
        G_DBUS_PROXY_FLAGS_NONE,
        NULL,
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Screenshot",
        nullptr,
        &error
    );

    // Check for errors
    if (proxy == NULL || error) {
        std::cerr << "Error creating proxy: " << error->message << std::endl;
        g_error_free(error);
        exit(EXIT_FAILURE);
    }
}

void take_screenshot()
{
    // Call the "Screenshot" method
    GVariant *result = g_dbus_proxy_call_sync(
        proxy,
        "Screenshot",
        g_variant_new("(sa{sv})", "screen"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error
    );

    // Check for errors
    if (error) {
        std::cerr << "Error taking screenshot: " << error->message << std::endl;
        g_error_free(error);
        g_object_unref(proxy);
        return;
    }

    if (!result) {
        std::cerr << "No result returned from the screenshot portal." << std::endl;
        return;
    }

    // Get uri where the portal saved the screenshot to and read it into a buffer

}

void destroy_portal()
{
    // Clean up
    g_object_unref(proxy);
}
