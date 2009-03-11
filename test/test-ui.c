#include "gpds-ui.h"

#include <gcutter.h>

void test_names (void);
void test_new (void);
void test_device_name (void);
void test_is_available (void);
void test_build (void);
void test_get_content_widget (void);
void test_get_label_widget (void);

static GError *error;
static GpdsUI *ui;
static GList *names;
static GList *expected_names;
static GtkWidget *widget;

#define DEVICE_NAME "Macintosh mouse button emulation"

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
    widget = NULL;
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

    if (error)
        g_clear_error(&error);
}

void
test_names (void)
{
    names = gpds_uis_get_names();

    cut_assert(names);
}

void
test_new (void)
{
    ui = gpds_ui_new("mouse",
                     "device-name", DEVICE_NAME,
                     NULL);
    cut_assert(ui);
}

void
test_device_name (void)
{
    cut_trace(test_new());

    cut_assert_equal_string(DEVICE_NAME, gpds_ui_get_device_name(ui));
}

void
test_is_available (void)
{
    gboolean available;

    cut_trace(test_new());

    available = gpds_ui_is_available(ui, &error);
    if (available)
        gcut_assert_error(error);
}

void
test_build (void)
{
    gboolean available;

    cut_trace(test_new());

    available = gpds_ui_is_available(ui, &error);
    if (!available)
        cut_omit("No %s.", DEVICE_NAME);
    cut_assert_true(gpds_ui_build(ui, &error));
    gcut_assert_error(error);
}

void
test_get_content_widget (void)
{
    cut_trace(test_build());

    widget = gpds_ui_get_content_widget(ui, &error);
    gcut_assert_error(error);
    cut_assert_true(GTK_IS_WIDGET(widget));
}

void
test_get_label_widget (void)
{
    cut_trace(test_build());

    widget = gpds_ui_get_label_widget(ui, &error);
    gcut_assert_error(error);
    cut_assert_true(GTK_IS_WIDGET(widget));
}



/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
