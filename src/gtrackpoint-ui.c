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

#include "gtrackpoint-ui.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gxinput.h"

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

typedef struct _GTrackPointUIPriv GTrackPointUIPriv;
struct _GTrackPointUIPriv
{
    GtkBuilder *builder;
    GError *error;
    GXInput *xinput;
};


#define G_TRACK_POINT_UI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), G_TYPE_TRACK_POINT_UI, GTrackPointUIPriv))

G_DEFINE_TYPE (GTrackPointUI, g_track_point_ui, G_TYPE_OBJECT)

static void dispose      (GObject      *object);

static void
g_track_point_ui_class_init (GTrackPointUIClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = dispose;

    g_type_class_add_private(gobject_class, sizeof(GTrackPointUIPriv));
}

static void
g_track_point_ui_init (GTrackPointUI *ui)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(ui);

    priv->xinput = NULL;
    priv->error = NULL;

    priv->builder = gtk_builder_new();
    gtk_builder_add_from_file(priv->builder, 
                              DATADIR "/gtrackpoint.ui", &priv->error);
    if (priv->error) {
        g_print("%s\n", priv->error->message);
    }
}

static void
dispose (GObject *object)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(object);

    if (priv->builder) {
        g_object_unref(priv->builder);
        priv->builder = NULL;
    }

    if (priv->xinput) {
        g_object_unref(priv->xinput);
        priv->xinput = NULL;
    }

    if (priv->error) {
        g_error_free(priv->error);
        priv->error = NULL;
    }

    if (G_OBJECT_CLASS(g_track_point_ui_parent_class)->dispose)
        G_OBJECT_CLASS(g_track_point_ui_parent_class)->dispose(object);
}

static void
show_error (GError *error)
{
    if (!error)
        return;

    g_print("%s\n", error->message);
}

static void
set_toggle_property (GXInput *xinput, GtkToggleButton *button, const gchar *property_name)
{
    GError *error = NULL;
    gboolean active;

    active = gtk_toggle_button_get_active(button);
    g_xinput_set_property(xinput,
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
set_spin_property (GXInput *xinput, GtkSpinButton *button, const gchar *property_name)
{
    GError *error = NULL;
    gdouble value;

    value = gtk_spin_button_get_value(button);
    g_xinput_set_property(xinput,
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
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(user_data);

    set_toggle_property(priv->xinput, button, MIDDLE_BUTTON_EMULATION);
    set_widget_sensitivity(priv->builder, "middle_button_emulation_box", button);
}

static void
cb_wheel_emulation_toggled (GtkToggleButton *button, gpointer user_data)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(user_data);

    set_toggle_property(priv->xinput, button, WHEEL_EMULATION);
    set_widget_sensitivity(priv->builder, "wheel_emulation_box", button);
}

static void
set_toggle_scroll_property (GXInput *xinput, GtkToggleButton *button,
                            const gchar *property_name,
                            gint first_value, gint second_value)
{
    GError *error = NULL;
    gboolean active;

    active = gtk_toggle_button_get_active(button);
    if (active) {
        g_xinput_set_property(xinput,
                              property_name,
                              &error,
                              first_value, second_value,
                              NULL);
    } else {
        g_xinput_set_property(xinput,
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
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(user_data);

    set_toggle_scroll_property(priv->xinput, button, WHEEL_EMULATION_Y_AXIS, 6, 7);
}

static void
cb_wheel_emulation_horizontal_toggled (GtkToggleButton *button, gpointer user_data)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(user_data);

    set_toggle_scroll_property(priv->xinput, button, WHEEL_EMULATION_X_AXIS, 4, 5);
}

static void
cb_wheel_emulation_timeout_value_changed (GtkSpinButton *button, gpointer user_data)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(user_data);

    set_spin_property(priv->xinput, button, WHEEL_EMULATION_TIMEOUT);
}

static void
cb_middle_button_timeout_value_changed (GtkSpinButton *button, gpointer user_data)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(user_data);

    set_spin_property(priv->xinput, button, MIDDLE_BUTTON_TIMEOUT);
}

static void
setup_signals (GTrackPointUI *ui)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(ui);
    GObject *object;

#define CONNECT(object_name, signal_name)                               \
    object = gtk_builder_get_object(priv->builder, #object_name);       \
    g_signal_connect(object, #signal_name,                              \
                     G_CALLBACK(cb_ ## object_name ## _ ## signal_name),\
                     ui)

    CONNECT(middle_button_emulation, toggled);
    CONNECT(middle_button_timeout, value_changed);
    CONNECT(wheel_emulation, toggled);
    CONNECT(wheel_emulation_timeout, value_changed);
    CONNECT(wheel_emulation_vertical, toggled);
    CONNECT(wheel_emulation_horizontal, toggled);

#undef CONNECT
}

static gboolean
get_integer_property (GXInput *xinput, const gchar *property_name,
                      gint **values, gulong *n_values)
{
    GError *error = NULL;

    g_xinput_get_property(xinput,
                          property_name,
                          &error,
                          values, n_values);
    if (error) {
        show_error(error);
        g_error_free(error);
        return FALSE;
    }

    return TRUE;

}

static void
set_integer_property (GXInput *xinput, const gchar *property_name,
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
set_boolean_property (GXInput *xinput, const gchar *property_name,
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
set_scroll_property (GXInput *xinput, const gchar *property_name,
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
setup_current_values (GTrackPointUI *ui)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(ui);

    set_boolean_property(priv->xinput, MIDDLE_BUTTON_EMULATION,
                         priv->builder, "middle_button_emulation");
    set_boolean_property(priv->xinput, WHEEL_EMULATION,
                         priv->builder, "wheel_emulation");

    set_integer_property(priv->xinput, MIDDLE_BUTTON_TIMEOUT,
                         priv->builder, "middle_button_timeout");
    set_integer_property(priv->xinput, WHEEL_EMULATION_TIMEOUT,
                         priv->builder, "wheel_emulation_timeout");

    set_scroll_property(priv->xinput, WHEEL_EMULATION_Y_AXIS,
                        priv->builder, "wheel_emulation_vertical");
    set_scroll_property(priv->xinput, WHEEL_EMULATION_X_AXIS,
                        priv->builder, "wheel_emulation_horizontal");
}

static void
setup (GTrackPointUI *ui)
{
    setup_current_values(ui);
    setup_signals(ui);
}

GTrackPointUI *
g_track_point_ui_new (void)
{
    return g_object_new(G_TYPE_TRACK_POINT_UI, NULL);
}

GtkWidget *
g_track_point_ui_get_widget (GTrackPointUI *ui)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(ui);

    if (!priv->builder)
        return NULL;

    if (!g_xinput_exist_device(DEVICE_NAME)) {
        g_print("No %s found\n", DEVICE_NAME);
        return NULL;
    }

    priv->xinput = g_xinput_new(DEVICE_NAME);
    setup(ui);

    return GTK_WIDGET(gtk_builder_get_object(priv->builder, "main-widget"));
}

GtkWidget *
g_track_point_ui_get_label_widget (GTrackPointUI *ui)
{
    GTrackPointUIPriv *priv = G_TRACK_POINT_UI_GET_PRIVATE(ui);

    if (!priv->builder)
        return NULL;

    return GTK_WIDGET(gtk_builder_get_object(priv->builder, "main-widget-label"));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
