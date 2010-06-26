#include "gpds-xinput-utils.h"

#include <gcutter.h>
#include <gdk/gdkx.h>

void test_exist_device (void);
void test_get_float_atom (void);
void test_get_device_info (void);
void test_get_device_info_from_id (void);
void test_open_device (void);
void test_open_no_device (void);
void test_get_device_num_buttons (void);

static GError *error;
static GError *expected_error;
static XDevice *device;

#define DEVICE_NAME "Macintosh mouse button emulation"

void
setup (void)
{
    error = NULL;
    device = NULL;
    expected_error = NULL;
}

void
teardown (void)
{
    if (device)
        XCloseDevice(GDK_DISPLAY(), device);
    g_clear_error(&error);
    g_clear_error(&expected_error);
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
    device_info = gpds_xinput_utils_get_device_info(DEVICE_NAME, &error);
    cut_assert(device_info);

    gcut_assert_error(error);
}

void
test_get_device_info_from_id (void)
{
    XDeviceInfo *device_info = NULL;
    device_info = gpds_xinput_utils_get_device_info(DEVICE_NAME, &error);
    cut_assert(device_info);

    gcut_assert_error(error);

    device_info = gpds_xinput_utils_get_device_info_from_id(device_info->id, &error);
    cut_assert_equal_string(DEVICE_NAME, device_info->name);
}

void
test_open_device (void)
{
    device = gpds_xinput_utils_open_device(DEVICE_NAME, &error);
    cut_assert(device);

    gcut_assert_error(error);
}

void
test_open_no_device (void)
{
    expected_error = g_error_new(GPDS_XINPUT_UTILS_ERROR,
                                 GPDS_XINPUT_UTILS_ERROR_NO_DEVICE,
                                 "No Invalid device found.");
    device = gpds_xinput_utils_open_device("Invalid device", &error);

    gcut_assert_equal_error(expected_error, error);
}

void
test_get_device_num_buttons (void)
{
    gshort n_buttons;

    n_buttons = gpds_xinput_utils_get_device_num_buttons(DEVICE_NAME, &error);
    gcut_assert_error(error);

    cut_assert_equal_int(5, n_buttons);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
