#include "gpds-xinput-utils.h"

#include <gcutter.h>
#include <gdk/gdkx.h>

void test_exist_device (void);
void test_get_float_atom (void);
void test_get_device_info (void);
void test_open_device (void);

static GError *error;
static XDevice *device;

void
setup (void)
{
    error = NULL;
    device = NULL;
}

void
teardown (void)
{
    if (device)
        XCloseDevice(GDK_DISPLAY(), device);
    g_clear_error(&error);
}

void
test_exist_device (void)
{
    cut_assert_false(gpds_xinput_utils_exist_device("There is no device."));
}

void
test_get_float_atom (void)
{
    gpds_xinput_utils_get_float_atom(&error);
    gcut_assert_error(error);
}

void
test_get_device_info (void)
{
    XDeviceInfo *device_info = NULL; 
    device_info = gpds_xinput_utils_get_device_info("Macintosh mouse button emulation");
    cut_assert(device_info);
}

void
test_open_device (void)
{
    device = gpds_xinput_utils_open_device ("Macintosh mouse button emulation", &error);
    cut_assert(device);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
