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

#define DEVICE_NAME "SynPS/2 Synaptics TouchPad"

#define EDGES "Synaptics Edges"
#define FINGER "Synaptics Finger"
#define TAP_TIME "Synaptics Tap Time"
#define TAP_MOVE "Synaptics Tap Move"
#define TAP_DURATIONS "Synaptics Tap Durations"
#define TAP_FAST_TAP "Synaptics Tap FastTap"
#define MIDDLE_BUTTON_TIMEOUT "Synaptics Middle Button Timeout"
#define TWO_FINGER_PRESSURE "Synaptics Two-Finger Pressure"
#define SCROLLING_DISTANCE "Synaptics Scrolling Distance"
#define EDGE_SCROLLING "Synaptics Edge Scrolling"
#define TWO_FINGER_SCROLLING "Synaptics Two-Finger Scrolling"
#define EDGE_MOTION_PRESSURE "Synaptics Edge Motion Pressure"
#define EDGE_MOTION_SPEED "Synaptics Edge Motion Speed"
#define EDGE_MOTION_ALWAYS "Synaptics Edge Motion Always"
#define BUTTON_SCROLLING "Synaptics Button Scrolling"
#define BUTTON_SCROLLING_REPEAT "Synaptics Button Scrolling Repeat"
#define SCROLLING_TIME "Synaptics Button Scrolling Time"
#define OFF "Synaptics Off"
#define GUESTMOUSE_OFF "Synaptics Guestmouse Off"
#define LOCKED_DRAGS "Synaptics Locked Drags"
#define LOCKED_DRAGS_TIMEOUT "Synaptics Locked Drags Timeout"
#define TAP_ACTION "Synaptics Tap Action"
#define CLICK_ACTION "Synaptics Click Action"
#define CIRCULAR_SCROLLING "Synaptics Circular Scrolling"
#define CIRCULAR_SCROLLING_TRIGGER "Synaptics Circular Scrolling Trigger"
#define CIRCULAR_PAD "Synaptics Circular Pad"
#define PALM_DETECTION "Synaptics Palm Detection"
#define PALM_DIMENSIONS "Synaptics Palm Dimensions"
#define PRESSURE_MOTION "Synaptics Pressure Motion"
#define GRAB_EVENT_DEVICE "Synaptics Grab Event Device"

static const gchar *touchpad_device_names[] =
{
    "SynPS/2 Synaptics TouchPad",
    "AlpsPS/2 ALPS GlidePoint"
};

static const gint n_touchpad_device_names = G_N_ELEMENTS(touchpad_device_names);

#define GPDS_TYPE_TOUCHPAD_UI            gpds_type_touchpad_ui
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
};

struct _GpdsTouchpadUIClass
{
    GpdsUIClass parent_class;
};

static GType gpds_type_touchpad_ui = 0;
static GpdsUIClass *parent_class;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GtkWidget *get_label_widget   (GpdsUI  *ui, GError **error);

static void
class_init (GpdsTouchpadUIClass *klass)
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
init (GpdsTouchpadUI *ui)
{
    ui->device_name = NULL;
    ui->xinput = NULL;
    ui->ui_file_path = 
        g_build_filename(get_ui_file_directory(), "touchpad.ui", NULL);
}

static void
register_type (GTypeModule *type_module)
{
    static const GTypeInfo info =
        {
            sizeof (GpdsTouchpadUIClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof(GpdsTouchpadUI),
            0,
            (GInstanceInitFunc) init,
        };

    gpds_type_touchpad_ui =
        g_type_module_register_type(type_module,
                                    GPDS_TYPE_UI,
                                    "GpdsTouchpadUI",
                                    &info, 0);
}

G_MODULE_EXPORT GList *
GPDS_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    GList *registered_types = NULL;

    register_type(type_module);
    if (gpds_type_touchpad_ui)
        registered_types =
            g_list_prepend(registered_types,
                           (gchar *)g_type_name(gpds_type_touchpad_ui));

    return registered_types;
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
    g_free(ui->device_name);
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
set_spin_property (GpdsXInput *xinput, GtkSpinButton *button, const gchar *property_name)
{
    GError *error = NULL;
    gdouble value;

    value = gtk_spin_button_get_value(button);
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
cb_faster_tapping_check_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, TAP_FAST_TAP);
}

static void
cb_circular_scroll_check_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, CIRCULAR_SCROLLING);
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

    CONNECT(faster_tapping_check, toggled);
    CONNECT(circular_scroll_check, toggled);

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
    GpdsTouchpadUI *touchpad_ui = GPDS_TOUCHPAD_UI(ui);

    set_boolean_property(touchpad_ui->xinput, TAP_FAST_TAP,
                         builder, "faster_tapping_check");
    set_boolean_property(touchpad_ui->xinput, CIRCULAR_SCROLLING,
                         builder, "circular_scroll_check");
}

static const gchar *
find_device_name (void)
{
    gint i;

    for (i = 0; i < n_touchpad_device_names; i++) {
        if (gpds_xinput_exist_device(touchpad_device_names[i]));
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
                    _("No  device found."));
        return FALSE;
    }

    if (!g_file_test(GPDS_TOUCHPAD_UI(ui)->ui_file_path, G_FILE_TEST_EXISTS)) {
        g_set_error(error,
                    GPDS_UI_ERROR,
                    GPDS_UI_ERROR_NO_UI_FILE,
                    _("%s does not found."),
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
