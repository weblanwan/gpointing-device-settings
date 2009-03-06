#include "gpds-xinput-utils.h"

#include <gcutter.h>
#include <gdk/gdkx.h>

void test_exist_device (void);
void test_get_float_atom (void);
void test_get_device_info (void);
void test_open_device (void);

static GError *error;
static XDevice *device;
static GList *pointer_infos;
static GList *expected_pointer_infos;

void
setup (void)
{
    error = NULL;
    device = NULL;
    pointer_infos = NULL;
    expected_pointer_infos = NULL;
}

void
teardown (void)
{
    if (device)
        XCloseDevice(GDK_DISPLAY(), device);
    g_clear_error(&error);

    if (pointer_infos) {
        g_list_foreach(pointer_infos, (GFunc)gpds_xinput_pointer_info_free, NULL);
        g_list_free(pointer_infos);
    }
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

static void
gpds_xinput_pointer_info_inspect (GString *string,
                                  gconstpointer data,
                                  gpointer user_data)
{
    GpdsXInputPointerInfo *info = (GpdsXInputPointerInfo*)data;

    g_string_append_printf(string, "%s (%s)", info->name, info->type_name);
}

static gboolean
gpds_xinput_pointer_info_equal (gconstpointer a, gconstpointer b)
{
    GpdsXInputPointerInfo *a_info = (GpdsXInputPointerInfo*)a;
    GpdsXInputPointerInfo *b_info = (GpdsXInputPointerInfo*)b;

    if (!a || !b)
        return FALSE;

    if (g_strcmp0(a_info->name, b_info->name))
        return FALSE;
    return (!g_strcmp0(a_info->type_name, b_info->type_name));
}

void
test_collect_pointer_infos (void)
{
    pointer_infos = gpds_xinput_utils_collect_pointer_infos();

    gcut_assert_equal_list(expected_pointer_infos, pointer_infos,
                           gpds_xinput_pointer_info_equal,
                           gpds_xinput_pointer_info_inspect,
                           NULL);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
