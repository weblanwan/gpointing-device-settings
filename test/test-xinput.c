#include "gxinput.h"

#include <gcutter.h>

void test_new (void);
void test_property (void);

static GXInput *xinput;
static gint *values;
static gulong n_values;
static GError *error;

void
setup (void)
{
    xinput = NULL;
    values = NULL;
    n_values = 0;

    error = NULL;
}

void
teardown (void)
{
    if (xinput)
        g_object_unref(xinput);
    g_free(values);
}

void
test_new (void)
{
    xinput = g_xinput_new("TPPS/2 IBM TrackPoint");
    cut_assert(xinput);
}

void
test_property (void)
{
    gint original_value;
    cut_trace(test_new());

    cut_assert_true(g_xinput_get_property(xinput,
                                          "Middle Button Emulation",
                                          &error,
                                          &values, &n_values));
    gcut_assert_error(error);
    cut_assert_equal_int(1, n_values);
    original_value = values[0];

    cut_assert_true(g_xinput_set_property(xinput,
                                          "Middle Button Emulation",
                                          &error,
                                          1, NULL));
    gcut_assert_error(error);
    g_free(values);
    cut_assert_true(g_xinput_get_property(xinput,
                                          "Middle Button Emulation",
                                          &error,
                                          &values, &n_values));
    gcut_assert_error(error);
    cut_assert_equal_int(1, n_values);
    cut_assert_equal_int(1, values[0]);

    cut_assert_true(g_xinput_set_property(xinput,
                                          "Middle Button Emulation",
                                          &error,
                                          original_value, NULL));
    gcut_assert_error(error);

    g_free(values);
    cut_assert_true(g_xinput_get_property(xinput,
                                          "Middle Button Emulation",
                                          &error,
                                          &values, &n_values));
    gcut_assert_error(error);
    cut_assert_equal_int(1, n_values);
    cut_assert_equal_int(original_value, values[0]);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
