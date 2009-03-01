/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2009 Hiroyuki Ikezoe  <poincare@ikezoe.net>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gpointing-device-settings.h>
#include <gpds-xinput.h>
#include <gconf/gconf-client.h>

#include "gpds-touchpad-definitions.h"


static const gchar *touchpad_device_names[] =
{
    "SynPS/2 Synaptics TouchPad",
    "AlpsPS/2 ALPS GlidePoint"
};

static const gint n_touchpad_device_names = G_N_ELEMENTS(touchpad_device_names);

#define GPDS_TYPE_TOUCHPAD_UI            (gpds_touchpad_ui_get_type())
#define GPDS_TOUCHPAD_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_TOUCHPAD_UI, GpdsTouchpadUI))
#define GPDS_TOUCHPAD_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_TOUCHPAD_UI, GpdsTouchpadUIClass))
#define G_IS_TOUCHPAD_UI(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_TOUCHPAD_UI))
#define G_IS_TOUCHPAD_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_TOUCHPAD_UI))
#define GPDS_TOUCHPAD_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_TOUCHPAD_UI, GpdsTouchpadUIClass))

typedef struct _GpdsTouchpadUI GpdsTouchpadUI;
typedef struct _GpdsTouchpadUIClass GpdsTouchpadUIClass;

struct _GpdsTouchpadUI
{
    GpdsUI parent;
    GpdsXInput *xinput;
    gchar *ui_file_path;
    gchar *device_name;
    GConfClient *gconf;
};

struct _GpdsTouchpadUIClass
{
    GpdsUIClass parent_class;
};

GType gpds_touchpad_ui_get_type (void) G_GNUC_CONST;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GtkWidget *get_label_widget   (GpdsUI  *ui, GError **error);

G_DEFINE_DYNAMIC_TYPE(GpdsTouchpadUI, gpds_touchpad_ui, GPDS_TYPE_UI)

static void
gpds_touchpad_ui_class_init (GpdsTouchpadUIClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GpdsUIClass *ui_class = GPDS_UI_CLASS(klass);

    gobject_class->dispose = dispose;

    ui_class->is_available       = is_available;
    ui_class->build              = build;
    ui_class->get_content_widget = get_content_widget;
    ui_class->get_label_widget   = get_label_widget;
}

static void
gpds_touchpad_ui_class_finalize (GpdsTouchpadUIClass *klass)
{
}

static const gchar *
get_ui_file_directory (void)
{
    const gchar *dir;

    dir = g_getenv("GPDS_UI_DIR");
    return dir ? dir : GPDS_UIDIR;
}

static void
gpds_touchpad_ui_init (GpdsTouchpadUI *ui)
{
    ui->device_name = NULL;
    ui->xinput = NULL;
    ui->ui_file_path = 
        g_build_filename(get_ui_file_directory(), "touchpad.ui", NULL);
    ui->gconf = gconf_client_get_default();
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    if (GPDS_TYPE_TOUCHPAD_UI)
        return;

    gpds_touchpad_ui_register_type(type_module);
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
GPDS_MODULE_IMPL_INSTANTIATE (void)
{
    return g_object_new(GPDS_TYPE_TOUCHPAD_UI, NULL);
}

static void
dispose (GObject *object)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(object);

    if (ui->xinput) {
        g_object_unref(ui->xinput);
        ui->xinput = NULL;
    }

    if (ui->gconf) {
        g_object_unref(ui->gconf);
        ui->gconf = NULL;
    }

    g_free(ui->device_name);
    g_free(ui->ui_file_path);

    if (G_OBJECT_CLASS(gpds_touchpad_ui_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_touchpad_ui_parent_class)->dispose(object);
}

static void
show_error (GError *error)
{
    if (!error)
        return;

    g_print("%s\n", error->message);
}

static void
set_widget_sensitivity (GtkBuilder *builder,
                        const gchar *widget_id, 
                        GtkToggleButton *button)
{
    GObject *object;

    object = gtk_builder_get_object(builder, widget_id);
    gtk_widget_set_sensitive(GTK_WIDGET(object),
                             gtk_toggle_button_get_active(button));
}

static void
set_toggle_property (GpdsXInput *xinput, GtkToggleButton *button, const gchar *property_name)
{
    GError *error = NULL;
    gboolean active;

    active = gtk_toggle_button_get_active(button);
    if (!gpds_xinput_set_property(xinput,
                                  property_name,
                                  &error,
                                  active ? 1 : 0,
                                  NULL)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

static void
set_edge_scroll_toggle_property (GpdsXInput *xinput, GtkBuilder *builder)
{
    GError *error = NULL;
    gboolean vertical_scrolling_active;
    gboolean horizontal_scrolling_active;
    GObject *object;

    object = gtk_builder_get_object(builder, "vertical_scroll_check");
    vertical_scrolling_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object));
    set_widget_sensitivity(builder, "vertical_scroll_box", GTK_TOGGLE_BUTTON(object));

    object = gtk_builder_get_object(builder, "horizontal_scroll_check");
    horizontal_scrolling_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object));
    set_widget_sensitivity(builder, "horizontal_scroll_box", GTK_TOGGLE_BUTTON(object));

    if (!gpds_xinput_set_property(xinput,
                                  GPDS_TOUCHPAD_EDGE_SCROLLING,
                                  &error,
                                  vertical_scrolling_active ? 1 : 0,
                                  horizontal_scrolling_active ? 1 : 0,
                                  NULL)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

static void
set_range_property (GpdsXInput *xinput, GtkRange *range, const gchar *property_name)
{
    GError *error = NULL;
    gdouble value;

    value = gtk_range_get_value(range);
    if (!gpds_xinput_set_property(xinput,
                                  property_name,
                                  &error,
                                  (gint)value,
                                  NULL)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

static void
set_scrolling_distance_range_property (GpdsXInput *xinput, GtkBuilder *builder)
{
    GError *error = NULL;
    gdouble vertical_scrolling_distance;
    gdouble horizontal_scrolling_distance;
    GObject *object;

    object = gtk_builder_get_object(builder, "horizontal_scroll_scale");
    vertical_scrolling_distance = gtk_range_get_value(GTK_RANGE(object));

    object = gtk_builder_get_object(builder, "vertical_scroll_scale");
    horizontal_scrolling_distance = gtk_range_get_value(GTK_RANGE(object));

    if (!gpds_xinput_set_property(xinput,
                                  GPDS_TOUCHPAD_SCROLLING_DISTANCE,
                                  &error,
                                  (gint)vertical_scrolling_distance,
                                  (gint)horizontal_scrolling_distance,
                                  NULL)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

static void
cb_tapping_time_scale_value_changed (GtkRange *range, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gdouble time;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_range_property(ui->xinput, range, GPDS_TOUCHPAD_TAP_TIME);

    time = gtk_range_get_value(range);
    gconf_client_set_int(ui->gconf, GPDS_TOUCHPAD_TAP_TIME_KEY, (gint)time, NULL);
}

static void
cb_faster_tapping_check_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gboolean check;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, GPDS_TOUCHPAD_TAP_FAST_TAP);
    check = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TOUCHPAD_TAP_FAST_TAP_KEY, check, NULL);
}

static void
cb_circular_scroll_check_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gboolean check;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, GPDS_TOUCHPAD_CIRCULAR_SCROLLING);
    set_widget_sensitivity(builder, "circular_scroll_box", button);

    check = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY, check, NULL);
}

static void
cb_vertical_scroll_check_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gboolean check;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_edge_scroll_toggle_property(ui->xinput, builder);

    check = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY, check, NULL);
}

static void
cb_horizontal_scroll_check_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gboolean check;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_edge_scroll_toggle_property(ui->xinput, builder);

    check = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY, check, NULL);
}

static void
cb_vertical_scroll_scale_value_changed (GtkRange *range, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gdouble distance;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_scrolling_distance_range_property(ui->xinput, builder);

    distance = gtk_range_get_value(range);
    gconf_client_set_int(ui->gconf, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY, (gint)distance, NULL);
}

static void
cb_horizontal_scroll_scale_value_changed (GtkRange *range, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gdouble distance;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_scrolling_distance_range_property(ui->xinput, builder);

    distance = gtk_range_get_value(range);
    gconf_client_set_int(ui->gconf, GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY, (gint)distance, NULL);
}

static void
setup_signals (GpdsUI *ui, GtkBuilder *builder)
{
    GObject *object;

#define CONNECT(object_name, signal_name)                               \
    object = gtk_builder_get_object(builder, #object_name);             \
    g_signal_connect(object, #signal_name,                              \
                     G_CALLBACK(cb_ ## object_name ## _ ## signal_name),\
                     ui)

    CONNECT(tapping_time_scale, value_changed);
    CONNECT(faster_tapping_check, toggled);
    CONNECT(circular_scroll_check, toggled);
    CONNECT(vertical_scroll_check, toggled);
    CONNECT(vertical_scroll_scale, value_changed);
    CONNECT(horizontal_scroll_check, toggled);
    CONNECT(horizontal_scroll_scale, value_changed);

#undef CONNECT
}

static gboolean
get_integer_property (GpdsXInput *xinput, const gchar *property_name,
                      gint **values, gulong *n_values)
{
    GError *error = NULL;

    if (!gpds_xinput_get_property(xinput,
                                  property_name,
                                  &error,
                                  values, n_values)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
        return FALSE;
    }

    return TRUE;

}

static void
set_integer_property_from_preference (GpdsTouchpadUI *ui,
                                      const gchar *property_name,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gint value;
    gboolean dir_exists;

    if (!get_integer_property(ui->xinput, property_name,
                              &values, &n_values)) {
        return;
    }

    dir_exists = gconf_client_dir_exists(ui->gconf, GPDS_TOUCHPAD_GCONF_DIR, NULL);
    if (dir_exists)
        value = gconf_client_get_int(ui->gconf, gconf_key_name, NULL);
    else
        value = values[0];
    object = gtk_builder_get_object(builder, object_name);
    gtk_range_set_value(GTK_RANGE(object), value);
    g_free(values);
}

static void
set_boolean_property_from_preference (GpdsTouchpadUI *ui,
                                      const gchar *property_name,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable, dir_exists;

    if (!get_integer_property(ui->xinput, property_name,
                              &values, &n_values)) {
        return;
    }

    dir_exists = gconf_client_dir_exists(ui->gconf, GPDS_TOUCHPAD_GCONF_DIR, NULL);
    if (dir_exists)
        enable = gconf_client_get_bool(ui->gconf, gconf_key_name, NULL);
    else
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, object_name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    g_free(values);
}

static void
set_edge_scroll_property_from_preference (GpdsTouchpadUI *ui,
                                          GtkBuilder *builder)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable, dir_exists;

    if (!get_integer_property(ui->xinput, GPDS_TOUCHPAD_EDGE_SCROLLING,
                              &values, &n_values)) {
        return;
    }

    if (n_values != 3) {
        g_free(values);
        return;
    }

    dir_exists = gconf_client_dir_exists(ui->gconf, GPDS_TOUCHPAD_GCONF_DIR, NULL);
    if (dir_exists)
        enable = gconf_client_get_bool(ui->gconf, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY, NULL);
    else
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "vertical_scroll_check");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);

    if (dir_exists)
        enable = gconf_client_get_bool(ui->gconf, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY, NULL);
    else
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "horizontal_scroll_check");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);

    g_free(values);
}

static void
set_scroll_distance_property_from_preference (GpdsTouchpadUI *ui,
                                              GtkBuilder *builder)
{
    GObject *object;
    GError *error = NULL;
    gint *values;
    gulong n_values;
    gint distance;

    if (!get_integer_property(ui->xinput,
                              GPDS_TOUCHPAD_SCROLLING_DISTANCE,
                              &values, &n_values)) {
        return;
    }

    if (n_values != 2) {
        g_free(values);
        return;
    }

    distance = gconf_client_get_int(ui->gconf,
                                    GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY,
                                    &error);
    object = gtk_builder_get_object(builder, "vertical_scroll_scale");
    gtk_range_set_value(GTK_RANGE(object), error ? values[0] : distance);
    if (error)
        g_clear_error(&error);

    distance = gconf_client_get_int(ui->gconf,
                                    GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY,
                                    &error);
    object = gtk_builder_get_object(builder, "horizontal_scroll_scale");
    gtk_range_set_value(GTK_RANGE(object), error ? values[1] : distance);
    if (error)
        g_clear_error(&error);

    g_free(values);
}

static void
setup_current_values (GpdsUI *ui, GtkBuilder *builder)
{
    GpdsTouchpadUI *touchpad_ui = GPDS_TOUCHPAD_UI(ui);

    set_integer_property_from_preference(touchpad_ui,
                                         GPDS_TOUCHPAD_TAP_TIME,
                                         GPDS_TOUCHPAD_TAP_TIME_KEY,
                                         builder,
                                         "tapping_time_scale");
    set_boolean_property_from_preference(touchpad_ui,
                                         GPDS_TOUCHPAD_TAP_FAST_TAP, 
                                         GPDS_TOUCHPAD_TAP_FAST_TAP_KEY,
                                         builder,
                                         "faster_tapping_check");
    set_boolean_property_from_preference(touchpad_ui,
                                         GPDS_TOUCHPAD_CIRCULAR_SCROLLING,
                                         GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY, 
                                         builder,
                                         "circular_scroll_check");
    set_edge_scroll_property_from_preference(touchpad_ui, builder);
    set_scroll_distance_property_from_preference(touchpad_ui, builder);
}

static const gchar *
find_device_name (void)
{
    gint i;

    for (i = 0; i < n_touchpad_device_names; i++) {
        if (gpds_xinput_exist_device(touchpad_device_names[i]))
            return touchpad_device_names[i];
    }
    return NULL;
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    const gchar *device_name;
    device_name = find_device_name();

    if (!device_name) {
        g_set_error(error,
                    GPDS_XINPUT_ERROR,
                    GPDS_XINPUT_ERROR_NO_DEVICE,
                    _("No Touchpad device found."));
        return FALSE;
    }

    if (!g_file_test(GPDS_TOUCHPAD_UI(ui)->ui_file_path, G_FILE_TEST_EXISTS)) {
        g_set_error(error,
                    GPDS_UI_ERROR,
                    GPDS_UI_ERROR_NO_UI_FILE,
                    _("%s did not find."),
                    GPDS_TOUCHPAD_UI(ui)->ui_file_path);
        return FALSE;
    }

    GPDS_TOUCHPAD_UI(ui)->device_name = g_strdup(device_name);

    return TRUE;
}

static gboolean
build (GpdsUI  *ui, GError **error)
{
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(ui);

    if (!gtk_builder_add_from_file(builder, 
                                   GPDS_TOUCHPAD_UI(ui)->ui_file_path,
                                   error)) {
        return FALSE;
    }

    GPDS_TOUCHPAD_UI(ui)->xinput = gpds_xinput_new(GPDS_TOUCHPAD_UI(ui)->device_name);

    setup_current_values(ui, builder);
    setup_signals(ui, builder);

    return TRUE;
}

static GtkWidget *
get_content_widget (GpdsUI *ui, GError **error)
{
    GtkBuilder *builder;
    GObject *widget;

    builder = gpds_ui_get_builder(ui);

    widget = gtk_builder_get_object(builder, "main-widget");
    if (!widget) {
        g_set_error(error,
                    GPDS_UI_ERROR,
                    GPDS_UI_ERROR_NO_WIDGET,
                    _("There is no widget(%s)."),
                    "main-widget");
    }

    return GTK_WIDGET(widget);
}

static GtkWidget *
get_label_widget (GpdsUI *ui, GError **error)
{
    GtkBuilder *builder;
    GObject *widget;

    builder = gpds_ui_get_builder(ui);

    widget = gtk_builder_get_object(builder, "main-widget-label");
    if (!widget) {
        g_set_error(error,
                    GPDS_UI_ERROR,
                    GPDS_UI_ERROR_NO_WIDGET,
                    _("There is no widget(%s)."),
                    "main-widget-label");
    }

    return GTK_WIDGET(widget);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
