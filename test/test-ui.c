#include "gpds-ui.h"

#include <gcutter.h>

void test_new (void);
void test_is_available (void);
void test_build (void);

static GError *error;
static GpdsUI *ui;

void
setup (void)
{
    ui = NULL;
    error = NULL;
}

void
teardown (void)
{
    if (ui)
        g_object_unref(ui);
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
