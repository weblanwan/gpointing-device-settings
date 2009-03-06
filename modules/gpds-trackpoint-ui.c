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

#include "gpds-trackpoint-definitions.h"
#include "gpds-trackpoint-xinput.h"

#define GPDS_TYPE_TRACK_POINT_UI            (gpds_track_point_ui_get_type())
#define GPDS_TRACK_POINT_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_TRACK_POINT_UI, GpdsTrackPointUI))
#define GPDS_TRACK_POINT_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_TRACK_POINT_UI, GpdsTrackPointUIClass))
#define G_IS_TRACK_POINT_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_TRACK_POINT_UI))
#define G_IS_TRACK_POINT_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_TRACK_POINT_UI))
#define GPDS_TRACK_POINT_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_TRACK_POINT_UI, GpdsTrackPointUIClass))

typedef struct _GpdsTrackPointUI GpdsTrackPointUI;
typedef struct _GpdsTrackPointUIClass GpdsTrackPointUIClass;

struct _GpdsTrackPointUI
{
    GpdsUI parent;
    GpdsXInput *xinput;
    gchar *ui_file_path;
    GConfClient *gconf;
    gchar *device_name;
};

struct _GpdsTrackPointUIClass
{
    GpdsUIClass parent_class;
};

GType gpds_track_point_ui_get_type (void) G_GNUC_CONST;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GtkWidget *get_label_widget   (GpdsUI  *ui, GError **error);

G_DEFINE_DYNAMIC_TYPE(GpdsTrackPointUI, gpds_track_point_ui, GPDS_TYPE_UI)

static void
gpds_track_point_ui_class_init (GpdsTrackPointUIClass *klass)
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
gpds_track_point_ui_class_finalize (GpdsTrackPointUIClass *klass)
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
gpds_track_point_ui_init (GpdsTrackPointUI *ui)
{
    ui->xinput = NULL;
    ui->ui_file_path = g_build_filename(get_ui_file_directory(),
                                        "trackpoint.ui",
                                        NULL);
    ui->gconf = gconf_client_get_default();
    ui->device_name = NULL;
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    gpds_track_point_ui_register_type(type_module);
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
GPDS_MODULE_IMPL_INSTANTIATE (void)
{
    return g_object_new(GPDS_TYPE_TRACK_POINT_UI, NULL);
}

static void
dispose (GObject *object)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(object);

    if (ui->xinput) {
        g_object_unref(ui->xinput);
        ui->xinput = NULL;
    }

    if (ui->gconf) {
        g_object_unref(ui->gconf);
        ui->gconf = NULL;
    }

    g_free(ui->ui_file_path);
    g_free(ui->device_name);

    if (G_OBJECT_CLASS(gpds_track_point_ui_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_track_point_ui_parent_class)->dispose(object);
}

static void
show_error (GError *error)
{
    if (!error)
        return;

    g_print("%s\n", error->message);
}

static void
set_toggle_property (GpdsXInput *xinput, GtkToggleButton *button, GpdsTrackPointProperty property)
{
    GError *error = NULL;
    gboolean active;
    const gchar *property_name;
    gint format_type;
    gint properties[1];

    active = gtk_toggle_button_get_active(button);
    property_name = gpds_track_point_xinput_get_name(property);
    format_type = gpds_track_point_xinput_get_format_type(property);

    properties[0] = active ? 1 : 0;
    gpds_xinput_set_int_properties(xinput,
                                   property_name,
                                   format_type,
                                   &error,
                                   properties,
                                   1);
    if (error) {
        show_error(error);
        g_error_free(error);
    }
}

static void
set_spin_property (GpdsXInput *xinput, GtkSpinButton *button, GpdsTrackPointProperty property)
{
    GError *error = NULL;
    gdouble value;
    const gchar *property_name;
    gint format_type;
    gint properties[1];

    value = gtk_spin_button_get_value(button);
    property_name = gpds_track_point_xinput_get_name(property);
    format_type = gpds_track_point_xinput_get_format_type(property);

    properties[0] = (gint)value;
    gpds_xinput_set_int_properties(xinput,
                                   property_name,
                                   format_type,
                                   &error,
                                   properties,
                                   1);
    if (error) {
        show_error(error);
        g_error_free(error);
    }
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
cb_middle_button_emulation_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    GtkBuilder *builder;
    gboolean enable;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, GPDS_TRACK_POINT_MIDDLE_BUTTON_EMULATION);

    enable = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TRACK_POINT_MIDDLE_BUTTON_EMULATION_KEY, enable, NULL);
    set_widget_sensitivity(builder, "middle_button_emulation_box", button);
}

static void
cb_wheel_emulation_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    GtkBuilder *builder;
    gboolean enable;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, GPDS_TRACK_POINT_WHEEL_EMULATION);

    enable = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TRACK_POINT_WHEEL_EMULATION_KEY, enable, NULL);
    set_widget_sensitivity(builder, "wheel_emulation_box", button);
}

static void
set_scroll_axes_property (GpdsTrackPointUI *ui)
{
    GtkBuilder *builder;
    GtkToggleButton *button;
    GError *error = NULL;
    gboolean active;
    const gchar *property_name;
    gint format_type;
    gint properties[4];

    builder = gpds_ui_get_builder(GPDS_UI(ui));

    button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wheel_emulation_vertical"));
    active = gtk_toggle_button_get_active(button);
    if (active) {
        properties[0] = 6;
        properties[1] = 7;
    } else {
        properties[0] = 0;
        properties[1] = 0;
    }

    button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wheel_emulation_horizontal"));
    active = gtk_toggle_button_get_active(button);
    if (active) {
        properties[2] = 4;
        properties[3] = 5;
    } else {
        properties[2] = 0;
        properties[3] = 0;
    }

    property_name = gpds_track_point_xinput_get_name(GPDS_TRACK_POINT_WHEEL_EMULATION_AXES);
    format_type = gpds_track_point_xinput_get_format_type(GPDS_TRACK_POINT_WHEEL_EMULATION_AXES);

    gpds_xinput_set_int_properties(ui->xinput,
                                   property_name,
                                   format_type,
                                   &error,
                                   properties,
                                   4);
    if (error) {
        show_error(error);
        g_error_free(error);
    }
}

static void
cb_wheel_emulation_vertical_toggled (GtkToggleButton *button, gpointer user_data)
{
    gboolean enable;
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_scroll_axes_property(ui);

    enable = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TRACK_POINT_WHEEL_EMULATION_Y_AXIS_KEY, enable, NULL);
}

static void
cb_wheel_emulation_horizontal_toggled (GtkToggleButton *button, gpointer user_data)
{
    gboolean enable;
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_scroll_axes_property(ui);

    enable = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TRACK_POINT_WHEEL_EMULATION_X_AXIS_KEY, enable, NULL);
}

static void
cb_wheel_emulation_timeout_value_changed (GtkSpinButton *button, gpointer user_data)
{
    gdouble time;
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_spin_property(ui->xinput, button, GPDS_TRACK_POINT_WHEEL_EMULATION_TIMEOUT);

    time = gtk_spin_button_get_value(button);
    gconf_client_set_int(ui->gconf, GPDS_TRACK_POINT_WHEEL_EMULATION_TIMEOUT_KEY, (gint)time, NULL);
}

static void
cb_wheel_emulation_inertia_value_changed (GtkSpinButton *button, gpointer user_data)
{
    gdouble inertia;
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_spin_property(ui->xinput, button, GPDS_TRACK_POINT_WHEEL_EMULATION_INERTIA);

    inertia = gtk_spin_button_get_value(button);
    gconf_client_set_int(ui->gconf, GPDS_TRACK_POINT_WHEEL_EMULATION_INERTIA_KEY, (gint)inertia, NULL);
}

static void
cb_middle_button_timeout_value_changed (GtkSpinButton *button, gpointer user_data)
{
    gdouble time;
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_spin_property(ui->xinput, button, GPDS_TRACK_POINT_MIDDLE_BUTTON_TIMEOUT);

    time = gtk_spin_button_get_value(button);
    gconf_client_set_int(ui->gconf, GPDS_TRACK_POINT_MIDDLE_BUTTON_TIMEOUT_KEY, (gint)time, NULL);
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

    CONNECT(middle_button_emulation, toggled);
    CONNECT(middle_button_timeout, value_changed);
    CONNECT(wheel_emulation, toggled);
    CONNECT(wheel_emulation_timeout, value_changed);
    CONNECT(wheel_emulation_inertia, value_changed);
    CONNECT(wheel_emulation_vertical, toggled);
    CONNECT(wheel_emulation_horizontal, toggled);

#undef CONNECT
}

static gboolean
get_integer_properties (GpdsXInput *xinput, const gchar *property_name,
                        gint **values, gulong *n_values)
{
    GError *error = NULL;

    if (!gpds_xinput_get_int_properties(xinput,
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
set_integer_property_from_preference (GpdsTrackPointUI *ui,
                                      GpdsTrackPointProperty property,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gint value;
    gboolean dir_exists;
    const gchar *property_name;

    property_name = gpds_track_point_xinput_get_name(property);

    if (!get_integer_properties(ui->xinput, property_name,
                                &values, &n_values)) {
        return;
    }

    dir_exists = gconf_client_dir_exists(ui->gconf, GPDS_TRACK_POINT_GCONF_DIR, NULL);
    if (dir_exists)
        value = gconf_client_get_int(ui->gconf, gconf_key_name, NULL);
    else
        value = values[0];

    object = gtk_builder_get_object(builder, object_name);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(object), value);
    g_free(values);
}

static void
set_boolean_property_from_preference (GpdsTrackPointUI *ui,
                                      GpdsTrackPointProperty property,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable, dir_exists;
    const gchar *property_name;
    gchar *box_name;

    property_name = gpds_track_point_xinput_get_name(property);

    if (!get_integer_properties(ui->xinput, property_name,
                                &values, &n_values)) {
        return;
    }

    dir_exists = gconf_client_dir_exists(ui->gconf, GPDS_TRACK_POINT_GCONF_DIR, NULL);
    if (dir_exists)
        enable = gconf_client_get_bool(ui->gconf, gconf_key_name, NULL);
    else
        enable = (values[0] == 1);

    object = gtk_builder_get_object(builder, object_name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    g_free(values);

    box_name = g_strconcat(object_name, "_box", NULL);
    set_widget_sensitivity (builder, box_name, GTK_TOGGLE_BUTTON(object));
    g_free(box_name);
}

static void
set_scroll_axes_property_from_preference (GpdsTrackPointUI *ui,
                                          GtkBuilder *builder)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean horizontal_enable = FALSE, vertical_enable = FALSE, dir_exists;
    const gchar *property_name;

    property_name = gpds_track_point_xinput_get_name(GPDS_TRACK_POINT_WHEEL_EMULATION_AXES);

    if (!get_integer_properties(ui->xinput, property_name,
                                &values, &n_values)) {
        return;
    }

    dir_exists = gconf_client_dir_exists(ui->gconf, GPDS_TRACK_POINT_GCONF_DIR, NULL);
    if (dir_exists) {
        horizontal_enable = gconf_client_get_bool(ui->gconf,
                                                  GPDS_TRACK_POINT_WHEEL_EMULATION_X_AXIS_KEY,
                                                  NULL);
        vertical_enable = gconf_client_get_bool(ui->gconf,
                                                GPDS_TRACK_POINT_WHEEL_EMULATION_Y_AXIS_KEY,
                                                NULL);
    } else {
        if (n_values >= 2)
            horizontal_enable = (values[0] != 0 && values[1] != 0);
        if (n_values >= 4)
            vertical_enable = (values[2] != 0 && values[3] != 0);
    }

    object = gtk_builder_get_object(builder, "wheel_emulation_horizontal");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), horizontal_enable);
    object = gtk_builder_get_object(builder, "wheel_emulation_vertical");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), vertical_enable);

    g_free(values);
}

static void
setup_current_values (GpdsUI *ui, GtkBuilder *builder)
{
    GpdsTrackPointUI *track_point_ui = GPDS_TRACK_POINT_UI(ui);

    set_boolean_property_from_preference(track_point_ui,
                                         GPDS_TRACK_POINT_MIDDLE_BUTTON_EMULATION,
                                         GPDS_TRACK_POINT_MIDDLE_BUTTON_EMULATION_KEY,
                                         builder,
                                         "middle_button_emulation");
    set_boolean_property_from_preference(track_point_ui,
                                         GPDS_TRACK_POINT_WHEEL_EMULATION,
                                         GPDS_TRACK_POINT_WHEEL_EMULATION_KEY,
                                         builder,
                                         "wheel_emulation");
    set_integer_property_from_preference(track_point_ui,
                                         GPDS_TRACK_POINT_MIDDLE_BUTTON_TIMEOUT,
                                         GPDS_TRACK_POINT_MIDDLE_BUTTON_TIMEOUT_KEY,
                                         builder,
                                         "middle_button_timeout");
    set_integer_property_from_preference(track_point_ui,
                                         GPDS_TRACK_POINT_WHEEL_EMULATION_TIMEOUT,
                                         GPDS_TRACK_POINT_WHEEL_EMULATION_TIMEOUT_KEY,
                                         builder,
                                         "wheel_emulation_timeout");
    set_integer_property_from_preference(track_point_ui,
                                         GPDS_TRACK_POINT_WHEEL_EMULATION_INERTIA,
                                         GPDS_TRACK_POINT_WHEEL_EMULATION_INERTIA_KEY,
                                         builder,
                                         "wheel_emulation_inertia");

    set_scroll_axes_property_from_preference(track_point_ui,
                                             builder);
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    const gchar *device_name;
    device_name = gpds_track_point_xinput_find_device_name();

    if (!device_name) {
        g_set_error(error,
                    GPDS_XINPUT_ERROR,
                    GPDS_XINPUT_ERROR_NO_DEVICE,
                    _("No TrackPoint device found."));
        return FALSE;
    }

    if (!g_file_test(GPDS_TRACK_POINT_UI(ui)->ui_file_path,
                     G_FILE_TEST_EXISTS)) {
        g_set_error(error,
                    GPDS_UI_ERROR,
                    GPDS_UI_ERROR_NO_UI_FILE,
                    _("%s did not find."),
                    GPDS_TRACK_POINT_UI(ui)->ui_file_path);
        return FALSE;
    }

    GPDS_TRACK_POINT_UI(ui)->device_name = g_strdup(device_name);

    return TRUE;
}

static gboolean
build (GpdsUI  *ui, GError **error)
{
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(ui);

    if (!gtk_builder_add_from_file(builder, 
                                   GPDS_TRACK_POINT_UI(ui)->ui_file_path,
                                   error)) {
        return FALSE;
    }

    GPDS_TRACK_POINT_UI(ui)->xinput = gpds_xinput_new(GPDS_TRACK_POINT_UI(ui)->device_name);

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
