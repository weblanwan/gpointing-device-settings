#include "gpds-ui.h"

#include <gcutter.h>
#include "gpds-xinput.h"
#include "gpds-gconf.h"

void data_toggle_button (void);
void test_toggle_button (gconstpointer data);
void data_scale (void);
void test_scale (gconstpointer data);
void test_wheel_emulation_button (void);
void data_wheel_axes (void);
void test_wheel_axes (gconstpointer data);

static GError *error;
static GpdsUI *ui;
static GtkWidget *widget;
static GtkWidget *window;
static GpdsXInput *xinput;
static gint *values;

static gboolean wheel_emulation;
static gboolean middle_button_emulation;
static gboolean wheel_emulation_box_sensitivity;
static gboolean middle_button_emulation_box_sensitivity;
static gint wheel_emulation_button;
static gint wheel_emulation_timeout;
static gint wheel_emulation_inertia;
static gint wheel_emulation_axes[4];

#define DEVICE_NAME "Macintosh mouse button emulation"

static void
set_int_property_of_xinput (const gchar *property_name,
                            gint format_type,
                            gint *values,
                            guint n_values)
{
    xinput = gpds_xinput_new(DEVICE_NAME);
    gpds_xinput_set_int_properties_by_name_with_format_type(xinput,
                                                            property_name,
                                                            format_type,
                                                            &error,
                                                            values, n_values);
    gcut_assert_error(error);
    g_object_unref(xinput);
    xinput = NULL;
}

void
cut_startup (void)
{
    ui = gpds_ui_new("mouse",
                     "device-name", DEVICE_NAME,
                     NULL);
    gpds_ui_build(ui, NULL);

    widget = gpds_ui_get_content_widget(ui, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_add(GTK_CONTAINER(window), widget);
    gtk_widget_show_all(window);
}

void
cut_shutdown (void)
{
    if (ui)
        g_object_unref(ui);
    if (window)
        gtk_widget_destroy(window);
    if (xinput)
        g_object_unref(xinput);
    g_free(values);
}

static gboolean
get_boolean_property_of_xinput (const gchar *property_name)
{
    gulong n_values;

    xinput = gpds_xinput_new(DEVICE_NAME);
    gpds_xinput_get_int_properties_by_name(xinput,
                                           property_name,
                                           &error,
                                           &values, &n_values);
    gcut_assert_error(error);

    g_object_unref(xinput);
    xinput = NULL;

    return values[0] != 0;
}

static gint
get_int_property_of_xinput (const gchar *property_name)
{
    gulong n_values;

    xinput = gpds_xinput_new(DEVICE_NAME);
    gpds_xinput_get_int_properties_by_name(xinput,
                                           property_name,
                                           &error,
                                           &values, &n_values);
    gcut_assert_error(error);

    g_object_unref(xinput);
    xinput = NULL;

    return values[0];
}

static gint
get_scroll_axes_property_of_xinput (void)
{
    gulong n_values;

    xinput = gpds_xinput_new(DEVICE_NAME);
    gpds_xinput_get_int_properties_by_name(xinput,
                                           "Evdev Wheel Emulation Axes",
                                           &error,
                                           &values, &n_values);
    gcut_assert_error(error);

    g_object_unref(xinput);
    xinput = NULL;

    return (gint)n_values;
}

static void
preserve_initial_values (void)
{
    gint i;

    wheel_emulation = 
        get_boolean_property_of_xinput("Evdev Wheel Emulation");
    middle_button_emulation = 
        get_boolean_property_of_xinput("Evdev Middle Button Emulation");
    wheel_emulation_timeout = 
        get_int_property_of_xinput("Evdev Wheel Emulation Timeout");
    wheel_emulation_button = 
        get_int_property_of_xinput("Evdev Wheel Emulation Button");
    wheel_emulation_inertia = 
        get_int_property_of_xinput("Evdev Wheel Emulation Inertia");

    get_scroll_axes_property_of_xinput();
    for (i = 0; i < 4; i++)
        wheel_emulation_axes[i] = values[i];
}

static void
restore_initial_values (void)
{
    set_int_property_of_xinput("Evdev Wheel Emulation", 8,
                               &wheel_emulation, 1);
    set_int_property_of_xinput("Evdev Middle Button Emulation", 8,
                               &middle_button_emulation, 1);
    set_int_property_of_xinput("Evdev Wheel Emulation Timeout", 16,
                               &wheel_emulation_timeout, 1);
    set_int_property_of_xinput("Evdev Wheel Emulation Button", 8,
                               &wheel_emulation_button, 1);
    set_int_property_of_xinput("Evdev Wheel Emulation Inertia", 16,
                               &wheel_emulation_inertia, 1);
    set_int_property_of_xinput("Evdev Wheel Emulation Axes", 8,
                               wheel_emulation_axes, 4);
}

static GObject *
get_object (const gchar *id)
{
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

    return gtk_builder_get_object(builder, id);
}

static GtkWidget *
get_widget (const gchar *id)
{
    return GTK_WIDGET(get_object(id));
}

static void
preserve_widget_sensitivities (void)
{
    wheel_emulation_box_sensitivity = 
        GTK_WIDGET_SENSITIVE(get_widget("wheel_emulation_box"));
    middle_button_emulation_box_sensitivity = 
        GTK_WIDGET_SENSITIVE(get_widget("middle_button_emulation_box"));
}

static void
restore_widget_sensitivities (void)
{
    gtk_widget_set_sensitive(get_widget("wheel_emulation_box"),
                             wheel_emulation_box_sensitivity);
    gtk_widget_set_sensitive(get_widget("middle_button_emulation_box"),
                             middle_button_emulation_box_sensitivity);
}

static void
unset_all_gconf_keys (void)
{
    GConfClient *gconf;
    gchar *escape_key, *gconf_key;

    escape_key = gconf_escape_key(DEVICE_NAME, -1);
    gconf_key = g_strdup_printf("%s/%s",
                                GPDS_GCONF_DIR,
                                escape_key);
    g_free(escape_key);

    gconf = gconf_client_get_default();
    gconf_client_recursive_unset(gconf, 
                                 gconf_key,
                                 0,
                                 NULL);
    g_free(gconf_key);
    g_object_unref(gconf);
}

void
setup (void)
{
    error = NULL;
    preserve_initial_values();
    preserve_widget_sensitivities();

    unset_all_gconf_keys();
}

void
teardown (void)
{
    unset_all_gconf_keys();
    restore_initial_values();
    restore_widget_sensitivities();

    if (error)
        g_clear_error(&error);
}

static gboolean
cb_idle (gpointer data)
{
    gboolean *idle_received = data;

    *idle_received = TRUE;

    return FALSE;
}

static void
wait_action (void)
{
    guint idle_id;
    gboolean idle_received = FALSE;

    idle_id = g_idle_add_full(G_PRIORITY_LOW,
                              cb_idle,
                              &idle_received, NULL);
    while (!idle_received)
        g_main_context_iteration(NULL, FALSE);
}

void
data_toggle_button (void)
{
    gcut_add_datum("wheel emulation",
                   "widget-name", G_TYPE_STRING, "wheel_emulation",
                   "xinput-name", G_TYPE_STRING, "Evdev Wheel Emulation",
                   "dependent-widget-name", G_TYPE_STRING, "wheel_emulation_box",
                   NULL);
    gcut_add_datum("middle button emulation",
                   "widget-name", G_TYPE_STRING, "middle_button_emulation",
                   "xinput-name", G_TYPE_STRING, "Evdev Middle Button Emulation",
                   "dependent-widget-name", G_TYPE_STRING, "middle_button_emulation_box",
                   NULL);
}

void
test_toggle_button (gconstpointer data)
{
    GtkWidget *button, *dependent_widget;
    gboolean widget_value;
    gboolean xinput_value;
    const gchar *widget_name;
    const gchar *xinput_name;
    const gchar *dependent_widget_name;

    widget_name = gcut_data_get_string(data, "widget-name");
    xinput_name = gcut_data_get_string(data, "xinput-name");
    dependent_widget_name = gcut_data_get_string(data, "dependent-widget-name");

    button = get_widget(widget_name);
    cut_assert_true(GTK_IS_TOGGLE_BUTTON(button));

    xinput_value = get_boolean_property_of_xinput(xinput_name);
    widget_value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    cut_assert_equal_int(xinput_value, widget_value);

    /* check widget sensitivity */
    dependent_widget = get_widget(dependent_widget_name);
    cut_assert_equal_int(widget_value, GTK_WIDGET_SENSITIVE(dependent_widget));

    gtk_test_widget_click(button, 1, 0);
    wait_action();
    xinput_value = get_boolean_property_of_xinput(xinput_name);
    cut_assert_equal_int(xinput_value, !widget_value);

    cut_assert_equal_int(!widget_value, GTK_WIDGET_SENSITIVE(dependent_widget));
}

void
data_scale (void)
{
    gcut_add_datum("wheel emulation timeout",
                   "widget-name", G_TYPE_STRING, "wheel_emulation_timeout_scale",
                   "xinput-name", G_TYPE_STRING, "Evdev Wheel Emulation Timeout",
                   "box-name", G_TYPE_STRING, "wheel_emulation_box",
                   NULL);
    gcut_add_datum("wheel emulation inertia",
                   "widget-name", G_TYPE_STRING, "wheel_emulation_inertia_scale",
                   "xinput-name", G_TYPE_STRING, "Evdev Wheel Emulation Inertia",
                   "box-name", G_TYPE_STRING, "wheel_emulation_box",
                   NULL);
    gcut_add_datum("middle button timeout",
                   "widget-name", G_TYPE_STRING, "middle_button_timeout_scale",
                   "xinput-name", G_TYPE_STRING, "Evdev Middle Button Timeout",
                   "box-name", G_TYPE_STRING, "middle_button_emulation_box",
                   NULL);
}

static void
enable_widget (const gchar *widget_name)
{
    GtkWidget *widget;

    widget = get_widget(widget_name);
    if (widget)
        gtk_widget_set_sensitive(widget, TRUE);
    wait_action();
}

void
test_scale (gconstpointer data)
{
    GtkWidget *scale;
    gint widget_value;
    gint xinput_value;
    const gchar *widget_name;
    const gchar *xinput_name;
    const gchar *box_name;

    widget_name = gcut_data_get_string(data, "widget-name");
    xinput_name = gcut_data_get_string(data, "xinput-name");
    box_name = gcut_data_get_string(data, "box-name");

    enable_widget(box_name);

    scale = get_widget(widget_name);
    cut_assert_true(GTK_IS_RANGE(scale));

    xinput_value = get_int_property_of_xinput(xinput_name);
    widget_value = gtk_test_slider_get_value(scale);
    cut_assert_equal_int(xinput_value, widget_value);

    gtk_test_slider_set_perc(scale, 50.0);
    wait_action();
    xinput_value = get_int_property_of_xinput(xinput_name);
    widget_value = gtk_test_slider_get_value(scale);
    cut_assert_equal_int(xinput_value, widget_value);
}

void
test_wheel_emulation_button (void)
{
    GtkWidget *combo;
    GObject *list_store;
    GtkTreeIter iter;
    GValue value = {0};
    gint widget_value;
    gint xinput_value;

    enable_widget("wheel_emulation_box");

    combo = get_widget("wheel_emulation_button");
    cut_assert_true(GTK_IS_COMBO_BOX(combo));

    xinput_value = get_int_property_of_xinput("Evdev Wheel Emulation Button");
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo), &iter);
    list_store = get_object("wheel_emulation_button_list_store");
    gtk_tree_model_get_value(GTK_TREE_MODEL(list_store),
                             &iter,
                             0,
                             &value);
    widget_value = g_value_get_int(&value);
    g_value_unset(&value);
    cut_assert_equal_int(xinput_value, widget_value);

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 9);
    wait_action();
    xinput_value = get_int_property_of_xinput("Evdev Wheel Emulation Button");
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo), &iter);
    gtk_tree_model_get_value(GTK_TREE_MODEL(list_store),
                             &iter,
                             0,
                             &value);
    widget_value = g_value_get_int(&value);
    g_value_unset(&value);
    cut_assert_equal_int(xinput_value, widget_value);
}

static void
_assert_equal_wheel_vertical_axis (gboolean enable)
{
    if (enable) {
        cut_assert_equal_int(6, values[0]);
        cut_assert_equal_int(7, values[1]);
    }
}

static void
_assert_equal_wheel_horizontal_axis (gboolean enable)
{
    if (enable) {
        cut_assert_equal_int(4, values[2]);
        cut_assert_equal_int(5, values[3]);
    }
}

void
data_wheel_axes (void)
{
    gcut_add_datum("vertial",
                   "widget-name", G_TYPE_STRING, "wheel_emulation_vertical",
                   "assert-function", G_TYPE_POINTER, _assert_equal_wheel_vertical_axis, NULL,
                   NULL);
    gcut_add_datum("horizontal",
                   "widget-name", G_TYPE_STRING, "wheel_emulation_horizontal",
                   "assert-function", G_TYPE_POINTER, _assert_equal_wheel_horizontal_axis, NULL,
                   NULL);
}

typedef void (*WheelAxisAssertFunction) (gboolean enable);

void
test_wheel_axes (gconstpointer data)
{
    GtkWidget *button;
    gint widget_value;
    const gchar *widget_name;
    WheelAxisAssertFunction assert_function;

    widget_name = gcut_data_get_string(data, "widget-name");
    assert_function = gcut_data_get_pointer(data, "assert-function");

    enable_widget("wheel_emulation_box");

    button = get_widget(widget_name);
    cut_assert_true(GTK_IS_CHECK_BUTTON(button));

    cut_assert_equal_int(4, get_scroll_axes_property_of_xinput());
    widget_value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    assert_function(widget_value);

    gtk_test_widget_click(button, 1, 0);
    wait_action();
    cut_assert_equal_int(4, get_scroll_axes_property_of_xinput());
    widget_value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    assert_function(widget_value);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
