/*
 * File: wayland_mouse.cpp
 * Project: cs2-autoaccept-linux
 * Created Date: 2025-04-17 16:12:26
 * Author: 3urobeat
 *
 * Last Modified: 2025-04-18 15:11:25
 * Modified By: 3urobeat
 */

// Code taken from: https://stackoverflow.com/questions/5190921/simulating-absolute-mouse-movements-in-linux-using-uinput


#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>

#include "main.h"


int fd;
struct uinput_user_dev uidev;
struct input_event     ev;

void wl_mouse_cleanup(int signo)
{
    if(ioctl(fd, UI_DEV_DESTROY) < 0)
           die("error: cannot destroy uinput device\n");
    else printf("Destroyed uinput_user_dev\n\n");
    close(fd);
    exit(EXIT_SUCCESS);
}

void wl_get_mouse()
{
    if (signal(SIGINT,wl_mouse_cleanup) == SIG_ERR)
    {
        printf("Error: Failed to register signal handler!\n");
        exit(EXIT_FAILURE);
    }

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("Error: Failed to open input device");
        exit(EXIT_FAILURE);
    }

    if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT) < 0)
        die("error: ioctl");

    if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
        die("error: ioctl");

    if(ioctl(fd, UI_SET_EVBIT, EV_ABS) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_ABSBIT,ABS_X) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_ABSBIT, ABS_Y) < 0)
        die("error: ioctl");

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    uidev.absmin[ABS_X]=0;
    uidev.absmax[ABS_X]=1023;
    uidev.absfuzz[ABS_X]=0;
    uidev.absflat[ABS_X ]=0;
    uidev.absmin[ABS_Y]=0;
    uidev.absmax[ABS_Y]=767;
    uidev.absfuzz[ABS_Y]=0;
    uidev.absflat[ABS_Y ]=0;

    if(write(fd, &uidev, sizeof(uidev)) < 0)
        die("error: write0");

    if(ioctl(fd, UI_DEV_CREATE) < 0)
        die("error: ioctl");
}

// TODO: Test if this works with touchpads
void wl_set_mouse_pos(int x, int y)
{
    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time,NULL);
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
        die("Error: Failed to write ABS_X");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
        die("Error: Failed to sync ABS_X");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
        die("Error: Failed to write ABS_Y");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
        die("Error: Failed to sync ABS_Y");
}

void wl_mouse_click(int depressed)
{
    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time,NULL);
    ev.type = EV_KEY;
    ev.code = BTN_LEFT;
    ev.value = depressed;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
        die("Error: Failed to write BTN_LEFT");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
        die("Error: Failed to sync BTN_LEFT");
}
