#include "gpds-xinput.h"

#include <gcutter.h>

void test_new (void);
void test_device_name (void);
void test_set_int_properties_by_name (void);
void test_set_int_properties (void);
void test_set_float_properties (void);
void test_register_property_entries (void);
void test_set_int_properties_by_name_invalid_format_type (void);
void test_set_int_properties_invalid_n_values (void);
void test_set_int_properties_invalid_property_enum (void);
void test_set_float_properties_fail (void);
void test_get_float_properties_fail (void);
void test_set_button_map (void);
void test_get_button_map (void);

static GpdsXInput *xinput;
static GpdsXInput *tmp_xinput;
static gint *values;
static gulong n_values;
static guchar *actual_map;
static gshort actual_n_buttons;
static GError *error;
static GError *expected_error;
static gboolean middle_button_emulation;

#define DEVICE_NAME "Macintosh mouse button emulation"

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

static gboolean
get_boolean_property_of_xinput (const gchar *property_name)
{
    gulong n_values;
    GError *local_error = NULL;

    tmp_xinput = gpds_xinput_new(DEVICE_NAME);
    gpds_xinput_get_int_properties_by_name(tmp_xinput,
                                           property_name,
                                           &local_error,
                                           &values, &n_values);
    gcut_assert_error(local_error);

    g_object_unref(tmp_xinput);
    tmp_xinput = NULL;

    return values[0] != 0;
}

static void
set_int_property_of_xinput (const gchar *property_name,
                            gint format_type,
                            gint *values,
                            guint n_values)
{
    GError *local_error = NULL;
    tmp_xinput = gpds_xinput_new(DEVICE_NAME);
    gpds_xinput_set_int_properties_by_name_with_format_type(tmp_xinput,
                                                            property_name,
                                                            format_type,
                                                            &local_error,
                                                            values, n_values);
    gcut_assert_error(local_error);
    g_object_unref(tmp_xinput);
    tmp_xinput = NULL;
}

void
setup (void)
{
    xinput = NULL;
    tmp_xinput = NULL;
    values = NULL;
    n_values = 0;
    actual_map = NULL;

    error = NULL;
    expected_error = NULL;
    middle_button_emulation = 
        get_boolean_property_of_xinput("Evdev Middle Button Emulation");
}

void
teardown (void)
{
    if (xinput)
        g_object_unref(xinput);
    if (tmp_xinput)
        g_object_unref(tmp_xinput);
    g_free(values);
    g_free(actual_map);

    if (expected_error)
        g_clear_error(&expected_error);
    set_int_property_of_xinput("Evdev Middle Button Emulation", 8,
                               &middle_button_emulation, 1);
}

void
test_new (void)
{
    xinput = gpds_xinput_new(DEVICE_NAME);
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

    cut_assert_equal_string(DEVICE_NAME,
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
test_set_float_properties_fail (void)
{
    gdouble double_value[1];
    expected_error = g_error_new(GPDS_XINPUT_ERROR,
                                 GPDS_XINPUT_ERROR_X_ERROR,
                                 "An X error occurred. "
                                 "The error was BadMatch (invalid parameter attributes).");

    cut_trace(test_register_property_entries());

    double_value[0] = 1.1;
    gpds_xinput_set_float_properties(xinput,
                                     GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                     &error,
                                     double_value,
                                     1);
    gcut_assert_equal_error(expected_error, error);
}

void
test_get_float_properties_fail (void)
{
    gdouble *double_value;
    expected_error = g_error_new(GPDS_XINPUT_ERROR,
                                 GPDS_XINPUT_ERROR_FORMAT_TYPE_MISMATCH,
                                 "Format type is mismatched.\n"
                                 "FLOAT is specified but INTEGER returns.");

    cut_trace(test_register_property_entries());

    gpds_xinput_get_float_properties(xinput,
                                     GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                     &error,
                                     &double_value,
                                     &n_values);
    gcut_assert_equal_error(expected_error, error);
}

void
test_set_int_properties_invalid_property_enum (void)
{
    gint invalid_values[2];
    expected_error = g_error_new(GPDS_XINPUT_ERROR,
                                 GPDS_XINPUT_ERROR_NO_REGISTERED_PROPERTY,
                                 "There is no registered property for %d.",
                                 GPDS_MOUSE_MIDDLE_BUTTON_EMULATION);

    cut_trace(test_new());
    gpds_xinput_set_int_properties(xinput,
                                   GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                   &error,
                                   invalid_values,
                                   2);
    gcut_assert_equal_error(expected_error, error);
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

void
test_get_button_map (void)
{
    const guchar expected_map[5] = { 1, 2, 3, 4, 5 };
    cut_trace(test_new());

    gpds_xinput_get_button_map (xinput, &error, &actual_map, &actual_n_buttons);
    gcut_assert_error(error);

    cut_assert_equal_int(5, actual_n_buttons);
    cut_assert_equal_memory(expected_map, sizeof(expected_map),
                            actual_map, actual_n_buttons);
}

void
test_set_button_map (void)
{
    test_get_button_map();

    gpds_xinput_set_button_map (xinput, &error, actual_map, actual_n_buttons);
    gcut_assert_error(error);

}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
