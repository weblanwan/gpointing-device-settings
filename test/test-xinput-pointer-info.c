#include "gpds-xinput-pointer-info.h"

#include <gcutter.h>
#include <gdk/gdkx.h>

void test_collect (void);
void test_name (void);
void test_type_name (void);

static GList *pointer_infos;
static GList *expected_pointer_infos;
static GpdsXInputPointerInfo *pointer_info;

void
setup (void)
{
    pointer_info = NULL;
    pointer_infos = NULL;
    expected_pointer_infos = NULL;
}

void
teardown (void)
{
    if (pointer_info) {
        gpds_xinput_pointer_info_free(pointer_info);
        pointer_info = NULL;
    }
    if (pointer_infos) {
        g_list_foreach(pointer_infos, (GFunc)gpds_xinput_pointer_info_free, NULL);
        g_list_free(pointer_infos);
    }
    if (expected_pointer_infos) {
        g_list_free(expected_pointer_infos);
    }
}

static void
test_new (void)
{
    pointer_info = gpds_xinput_pointer_info_new("Name", "TypeName");
    cut_assert(pointer_info);
}

void
test_name (void)
{
    cut_trace(test_new());

    cut_assert_equal_string("Name", gpds_xinput_pointer_info_get_name(pointer_info));
}

void
test_type_name (void)
{
    cut_trace(test_new());

    cut_assert_equal_string("TypeName", gpds_xinput_pointer_info_get_type_name(pointer_info));
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
test_collect (void)
{
    pointer_infos = gpds_xinput_utils_collect_pointer_infos();
    expected_pointer_infos = g_list_copy(pointer_infos);

    gcut_assert_equal_list(expected_pointer_infos, pointer_infos,
                           gpds_xinput_pointer_info_equal,
                           gpds_xinput_pointer_info_inspect,
                           NULL);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
