#include "gpds-ui.h"

#include <gcutter.h>
#include "gpds-xinput.h"

void test_names (void);
void test_new (void);
void test_device_name (void);
void test_is_available (void);
void test_build (void);
void test_get_content_widget (void);
void test_get_icon_pixbuf (void);
void test_create_window (void);
void test_wheel_emulation (void);
void test_wheel_emulation_timeout (void);

static GError *error;
static GpdsUI *ui;
static GList *names;
static GList *expected_names;
static GtkWidget *widget;
static GdkPixbuf *pixbuf;
static GtkWidget *window;
static GpdsXInput *xinput;
static gint *values;

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
    pixbuf = NULL;
    window = NULL;
    xinput = NULL;
    values = NULL;
}

void
teardown (void)
{
    if (ui)
        g_object_unref(ui);
    g_list_free(names);
    g_list_free(expected_names);

    if (pixbuf)
        g_object_unref(pixbuf);
    if (error)
        g_clear_error(&error);
    if (window)
        gtk_widget_destroy(window);
    if (xinput)
        g_object_unref(xinput);
    g_free(values);
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
test_get_icon_pixbuf (void)
{
    cut_trace(test_build());

    pixbuf = gpds_ui_get_icon_pixbuf(ui, &error);
    gcut_assert_error(error);
    cut_assert_true(GDK_IS_PIXBUF(pixbuf));
}

static GtkWidget *
get_widget (const gchar *id)
{
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

    return GTK_WIDGET(gtk_builder_get_object(builder, id));
}

void
test_create_window (void)
{
    cut_trace(test_get_content_widget());
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_add(GTK_CONTAINER(window), widget);
    gtk_widget_show_all(window);
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

    return values[0];
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
test_wheel_emulation (void)
{
    GtkWidget *button;
    gboolean active;
    gboolean current_value;

    cut_trace(test_create_window());

    button = get_widget("wheel_emulation");
    cut_assert_true(GTK_IS_TOGGLE_BUTTON(button));

    current_value = get_boolean_property_of_xinput("Evdev Wheel Emulation");
    active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    cut_assert_equal_int(current_value, active);

    gtk_test_widget_click(button, 1, 0);
    wait_action();
    current_value = get_boolean_property_of_xinput("Evdev Wheel Emulation");
    cut_assert_equal_int(current_value, !active);
}

void
test_wheel_emulation_timeout (void)
{
    GtkWidget *button;
    gint widget_value;
    gint xinput_value;

    cut_trace(test_create_window());

    button = get_widget("wheel_emulation_timeout");
    cut_assert_true(GTK_IS_SPIN_BUTTON(button));

    xinput_value = get_int_property_of_xinput("Evdev Wheel Emulation Timeout");
    widget_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button));
    cut_assert_equal_int(xinput_value, widget_value);

    gtk_test_spin_button_click(GTK_SPIN_BUTTON(button), 1, FALSE);
    wait_action();
    xinput_value = get_int_property_of_xinput("Evdev Wheel Emulation Timeout");
    /* cut_assert_equal_int(xinput_value, widget_value + 50);*/
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
