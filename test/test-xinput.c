#include "gpds-xinput.h"

#include <gcutter.h>

void test_new (void);
void test_set_int_properties (void);

static GpdsXInput *xinput;
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
    xinput = gpds_xinput_new("TPPS/2 IBM TrackPoint");
    cut_assert(xinput);
}

void
test_set_int_properties (void)
{
    cut_trace(test_new());

    cut_assert_true(gpds_xinput_get_property(xinput,
                                             "Evdev Middle Button Emulation",
                                             &error,
                                             &values, &n_values));
    gcut_assert_error(error);
    cut_assert_equal_int(1, n_values);

    cut_assert_true(gpds_xinput_set_int_properties(xinput,
                                                   "Evdev Middle Button Emulation",
                                                   8,
                                                   &error,
                                                   (gint*)&values,
                                                   n_values));
    gcut_assert_error(error);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
