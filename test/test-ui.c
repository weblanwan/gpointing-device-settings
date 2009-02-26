#include "gpds-ui.h"

#include <gcutter.h>

void test_names (void);
void test_new (void);
void test_is_available (void);
void test_build (void);

static GError *error;
static GpdsUI *ui;
static GList *names;
static GList *expected_names;

void
cut_startup (void)
{
    gpds_uis_load();
}

void
cut_shutdown (void)
{
    gpds_uis_unload();
}

void
setup (void)
{
    ui = NULL;
    error = NULL;
    names = NULL;
    expected_names = NULL;
}

void
teardown (void)
{
    if (ui)
        g_object_unref(ui);
    g_list_free(names);
    g_list_free(expected_names);
}

void
test_names (void)
{
    expected_names = g_list_append(expected_names, "touchpad");
    expected_names = g_list_append(expected_names, "trackpoint");

    names = gpds_uis_get_names();

    gcut_assert_equal_list_string(expected_names, names);
}

void
test_new (void)
{
    ui = gpds_ui_new("touchpad");
    cut_assert(ui);
}

void
test_is_available (void)
{
    cut_trace(test_new());

    gpds_ui_is_available(ui, &error);
    gcut_assert_error(error);
}

void
test_build (void)
{
    cut_trace(test_new());

    cut_assert_true(gpds_ui_build(ui, &error));
    gcut_assert_error(error);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
