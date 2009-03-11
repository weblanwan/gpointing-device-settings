#include "gpds-xinput.h"

#include <gcutter.h>

void test_new (void);
void test_device_name (void);
void test_set_int_properties_by_name (void);
void test_set_int_properties (void);
void test_register_property_entries (void);
void test_set_int_properties_by_name_invalid_format_type (void);
void test_set_int_properties_invalid_n_values (void);

static GpdsXInput *xinput;
static gint *values;
static gulong n_values;
static GError *error;
static GError *expected_error;

typedef enum {
    GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
    GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT,
    GPDS_MOUSE_WHEEL_EMULATION,
    GPDS_MOUSE_WHEEL_EMULATION_INERTIA,
    GPDS_MOUSE_WHEEL_EMULATION_AXES,
    GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT,
    GPDS_MOUSE_WHEEL_EMULATION_BUTTON,
    GPDS_MOUSE_DRAG_LOCK_BUTTONS,
} GpdsMouseProperty;

static const GpdsXInputPropertyEntry entries[] = {
    {GPDS_MOUSE_MIDDLE_BUTTON_EMULATION, "Evdev Middle Button Emulation", G_TYPE_INT,  8, 1},
    {GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT,   "Evdev Middle Button Timeout",   G_TYPE_INT, 32, 1},
    {GPDS_MOUSE_WHEEL_EMULATION,         "Evdev Wheel Emulation",         G_TYPE_INT,  8, 1},
    {GPDS_MOUSE_WHEEL_EMULATION_INERTIA, "Evdev Wheel Emulation Inertia", G_TYPE_INT, 16, 1},
    {GPDS_MOUSE_WHEEL_EMULATION_AXES,    "Evdev Wheel Emulation Axes",    G_TYPE_INT,  8, 4},
    {GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT, "Evdev Wheel Emulation Timeout", G_TYPE_INT, 16, 1},
    {GPDS_MOUSE_WHEEL_EMULATION_BUTTON,  "Evdev Wheel Emulation Button",  G_TYPE_INT,  8, 1},
    {GPDS_MOUSE_DRAG_LOCK_BUTTONS,       "Evdev Drag Lock Buttons",       G_TYPE_INT,  8, 2}
};

static const gint n_entries = G_N_ELEMENTS(entries);

void
setup (void)
{
    xinput = NULL;
    values = NULL;
    n_values = 0;

    error = NULL;
    expected_error = NULL;
}

void
teardown (void)
{
    if (xinput)
        g_object_unref(xinput);
    g_free(values);

    if (expected_error)
        g_clear_error(&expected_error);
}

void
test_new (void)
{
    xinput = gpds_xinput_new("Macintosh mouse button emulation");
    cut_assert(xinput);
}

void
test_register_property_entries (void)
{
    cut_trace(test_new());
    gpds_xinput_register_property_entries(xinput, entries, n_entries);
}

void
test_device_name (void)
{
    cut_trace(test_new());

    cut_assert_equal_string("Macintosh mouse button emulation",
                            gpds_xinput_get_device_name(xinput));
}

void
test_set_int_properties_by_name (void)
{
    cut_trace(test_new());

    gpds_xinput_get_int_properties_by_name (xinput,
                                            "Evdev Middle Button Emulation",
                                            &error,
                                            &values, &n_values);
    gcut_assert_error(error);
    cut_assert_equal_int(1, n_values);

    gpds_xinput_set_int_properties_by_name_with_format_type
                                                  (xinput,
                                                   "Evdev Middle Button Emulation",
                                                   8,
                                                   &error,
                                                   (gint*)&values,
                                                   n_values);
    gcut_assert_error(error);
}

void
test_set_int_properties_by_name_invalid_format_type (void)
{
    expected_error = g_error_new(GPDS_XINPUT_ERROR,
                                 GPDS_XINPUT_ERROR_X_ERROR,
                                 "An X error occurred. "
                                 "The error was BadValue (integer parameter out of range for operation).");
    cut_trace(test_new());

    gpds_xinput_get_int_properties_by_name (xinput,
                                            "Evdev Middle Button Emulation",
                                            &error,
                                            &values, &n_values);
    gcut_assert_error(error);
    cut_assert_equal_int(1, n_values);

    gpds_xinput_set_int_properties_by_name_with_format_type
                                                  (xinput,
                                                   "Evdev Middle Button Emulation",
                                                   33,
                                                   &error,
                                                   (gint*)&values,
                                                   n_values);
    gcut_assert_equal_error(expected_error, error);
}

void
test_set_int_properties (void)
{
    cut_trace(test_register_property_entries());

    gpds_xinput_get_int_properties(xinput,
                                   GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                   &error,
                                   &values, &n_values);
    gcut_assert_error(error);
    cut_assert_equal_int(1, n_values);

    gpds_xinput_set_int_properties(xinput,
                                   GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                   &error,
                                   (gint*)&values,
                                   n_values);
    gcut_assert_error(error);
}

void
test_set_int_properties_invalid_n_values (void)
{
    gint invalid_values[2];
    expected_error = g_error_new(GPDS_XINPUT_ERROR,
                                 GPDS_XINPUT_ERROR_X_ERROR,
                                 "An X error occurred. "
                                 "The error was BadMatch (invalid parameter attributes).");

    cut_trace(test_register_property_entries());

    gpds_xinput_set_int_properties(xinput,
                                   GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                   &error,
                                   invalid_values,
                                   2);
    gcut_assert_equal_error(expected_error, error);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
