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

#include "gpds-ui.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gpds-xinput.h"
#include "gpds-module-impl.h"

#define DEVICE_NAME "TPPS/2 IBM TrackPoint"
#define MIDDLE_BUTTON_EMULATION "Middle Button Emulation"
#define MIDDLE_BUTTON_TIMEOUT "Middle Button Timeout"
#define WHEEL_EMULATION "Wheel Emulation"
#define WHEEL_EMULATION_INERTIA "Wheel Emulation Inertia"
#define WHEEL_EMULATION_X_AXIS "Wheel Emulation X Axis"
#define WHEEL_EMULATION_Y_AXIS "Wheel Emulation Y Axis"
#define WHEEL_EMULATION_TIMEOUT "Wheel Emulation Timeout"
#define WHEEL_EMULATION_BUTTON "Wheel Emulation Button"
#define DRAG_LOCK_BUTTONS "Drag Lock Buttons"

#define GPDS_TYPE_TRACK_POINT_UI            gpds_type_track_point_ui
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
};

struct _GpdsTrackPointUIClass
{
    GpdsUIClass parent_class;
};

static GType gpds_type_track_point_ui = 0;
static GpdsUIClass *parent_class;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GtkWidget *get_label_widget   (GpdsUI  *ui, GError **error);

static void
class_init (GpdsTrackPointUIClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GpdsUIClass *ui_class = GPDS_UI_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);
    gobject_class->dispose = dispose;

    ui_class->is_available       = is_available;
    ui_class->build              = build;
    ui_class->get_content_widget = get_content_widget;
    ui_class->get_label_widget   = get_label_widget;
}

static const gchar *
get_ui_file_directory (void)
{
    const gchar *dir;

    dir = g_getenv("GPDS_UI_DIR");
    return dir ? dir : GPDS_UIDIR;
}

static void
init (GpdsTrackPointUI *ui)
{
    ui->xinput = NULL;
    ui->ui_file_path = g_build_filename(get_ui_file_directory(),
                                        "trackpoint.ui",
                                        NULL);
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (GpdsTrackPointUIClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof(GpdsTrackPointUI),
            0,
            (GInstanceInitFunc) init,
        };

    gpds_type_track_point_ui =
        g_type_module_register_type(type_module,
                                    GPDS_TYPE_UI,
                                    "GpdsTrackPointUI",
                                    &info, 0);
}

G_MODULE_EXPORT GList *
GPDS_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    GList *registered_types = NULL;

    register_type(type_module);
    if (gpds_type_track_point_ui)
        registered_types =
            g_list_prepend(registered_types,
                           (gchar *)g_type_name(gpds_type_track_point_ui));

    return registered_types;
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
    g_free(ui->ui_file_path);

    if (G_OBJECT_CLASS(parent_class)->dispose)
        G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void
show_error (GError *error)
{
    if (!error)
        return;

    g_print("%s\n", error->message);
}

static void
set_toggle_property (GpdsXInput *xinput, GtkToggleButton *button, const gchar *property_name)
{
    GError *error = NULL;
    gboolean active;

    active = gtk_toggle_button_get_active(button);
    gpds_xinput_set_property(xinput,
                             property_name,
                             &error,
                             active ? 1 : 0,
                             NULL);
    if (error) {
        show_error(error);
        g_error_free(error);
    }
}

static void
set_spin_property (GpdsXInput *xinput, GtkSpinButton *button, const gchar *property_name)
{
    GError *error = NULL;
    gdouble value;

    value = gtk_spin_button_get_value(button);
    gpds_xinput_set_property(xinput,
                             property_name,
                             &error,
                             (gint)value,
                             NULL);
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

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, MIDDLE_BUTTON_EMULATION);
    set_widget_sensitivity(builder, "middle_button_emulation_box", button);
}

static void
cb_wheel_emulation_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, WHEEL_EMULATION);
    set_widget_sensitivity(builder, "wheel_emulation_box", button);
}

static void
set_toggle_scroll_property (GpdsXInput *xinput, GtkToggleButton *button,
                            const gchar *property_name,
                            gint first_value, gint second_value)
{
    GError *error = NULL;
    gboolean active;

    active = gtk_toggle_button_get_active(button);
    if (active) {
        gpds_xinput_set_property(xinput,
                              property_name,
                              &error,
                              first_value, second_value,
                              NULL);
    } else {
        gpds_xinput_set_property(xinput,
                              property_name,
                              &error,
                              -1, -1,
                              NULL);
    }

    if (error) {
        show_error(error);
        g_error_free(error);
    }
}

static void
cb_wheel_emulation_vertical_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_toggle_scroll_property(ui->xinput, button, WHEEL_EMULATION_Y_AXIS, 6, 7);
}

static void
cb_wheel_emulation_horizontal_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_toggle_scroll_property(ui->xinput, button, WHEEL_EMULATION_X_AXIS, 4, 5);
}

static void
cb_wheel_emulation_timeout_value_changed (GtkSpinButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_spin_property(ui->xinput, button, WHEEL_EMULATION_TIMEOUT);
}

static void
cb_wheel_emulation_inertia_value_changed (GtkSpinButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_spin_property(ui->xinput, button, WHEEL_EMULATION_INERTIA);
}

static void
cb_middle_button_timeout_value_changed (GtkSpinButton *button, gpointer user_data)
{
    GpdsTrackPointUI *ui = GPDS_TRACK_POINT_UI(user_data);
    set_spin_property(ui->xinput, button, MIDDLE_BUTTON_TIMEOUT);
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
set_integer_property (GpdsXInput *xinput, const gchar *property_name,
                      GtkBuilder *builder, const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;

    if (!get_integer_property(xinput, property_name,
                              &values, &n_values)) {
        return;
    }

    object = gtk_builder_get_object(builder, object_name);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(object),
                              values[0]);
    g_free(values);
}

static void
set_boolean_property (GpdsXInput *xinput, const gchar *property_name,
                      GtkBuilder *builder, const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;

    if (!get_integer_property(xinput, property_name,
                              &values, &n_values)) {
        return;
    }

    object = gtk_builder_get_object(builder, object_name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object),
                                 values[0] == 1 ? TRUE : FALSE);
    g_free(values);
}

static void
set_scroll_property (GpdsXInput *xinput, const gchar *property_name,
                     GtkBuilder *builder, const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;

    if (!get_integer_property(xinput, property_name,
                              &values, &n_values)) {
        return;
    }

    object = gtk_builder_get_object(builder, object_name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object),
                                 n_values == 2 ? TRUE : FALSE);
    g_free(values);
}

static void
setup_current_values (GpdsUI *ui, GtkBuilder *builder)
{
    GpdsTrackPointUI *track_point_ui = GPDS_TRACK_POINT_UI(ui);

    set_boolean_property(track_point_ui->xinput, MIDDLE_BUTTON_EMULATION,
                         builder, "middle_button_emulation");
    set_boolean_property(track_point_ui->xinput, WHEEL_EMULATION,
                         builder, "wheel_emulation");

    set_integer_property(track_point_ui->xinput, MIDDLE_BUTTON_TIMEOUT,
                         builder, "middle_button_timeout");
    set_integer_property(track_point_ui->xinput, WHEEL_EMULATION_TIMEOUT,
                         builder, "wheel_emulation_timeout");
    set_integer_property(track_point_ui->xinput, WHEEL_EMULATION_INERTIA,
                         builder, "wheel_emulation_inertia");

    set_scroll_property(track_point_ui->xinput, WHEEL_EMULATION_Y_AXIS,
                        builder, "wheel_emulation_vertical");
    set_scroll_property(track_point_ui->xinput, WHEEL_EMULATION_X_AXIS,
                        builder, "wheel_emulation_horizontal");
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    if (!gpds_xinput_exist_device(DEVICE_NAME)) {
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

    GPDS_TRACK_POINT_UI(ui)->xinput = gpds_xinput_new(DEVICE_NAME);

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
