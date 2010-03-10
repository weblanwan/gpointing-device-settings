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

#include "gpds-xinput-ui.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gpds-module.h"
#include "gpds-xinput-utils.h"

typedef struct _GpdsXInputUIPriv GpdsXInputUIPriv;
struct _GpdsXInputUIPriv
{
    GpdsXInput *xinput;
};

#define GPDS_XINPUT_UI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_XINPUT_UI, GpdsXInputUIPriv))

G_DEFINE_ABSTRACT_TYPE(GpdsXInputUI, gpds_xinput_ui, GPDS_TYPE_UI)

static void     dispose        (GObject *object);
static gboolean is_available   (GpdsUI  *ui, GError **error);
static gboolean dry_run        (GpdsUI  *ui, GError **error);
static void     finish_dry_run (GpdsUI  *ui, GError **error);

static void
gpds_xinput_ui_class_init (GpdsXInputUIClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GpdsUIClass *ui_class = GPDS_UI_CLASS(klass);

    gobject_class->dispose = dispose;

    ui_class->is_available   = is_available;
    ui_class->dry_run        = dry_run;
    ui_class->finish_dry_run = finish_dry_run;

    g_type_class_add_private(gobject_class, sizeof(GpdsXInputUIPriv));
}

static void
gpds_xinput_ui_init (GpdsXInputUI *ui)
{
    GpdsXInputUIPriv *priv = GPDS_XINPUT_UI_GET_PRIVATE(ui);

    priv->xinput = NULL;
}

static void
dispose (GObject *object)
{
    GpdsXInputUIPriv *priv = GPDS_XINPUT_UI_GET_PRIVATE(object);

    if (priv->xinput) {
        g_object_unref(priv->xinput);
        priv->xinput = NULL;
    }

    if (G_OBJECT_CLASS(gpds_xinput_ui_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_xinput_ui_parent_class)->dispose(object);
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    if (GPDS_UI_CLASS(gpds_xinput_ui_parent_class)->is_available &&
        !GPDS_UI_CLASS(gpds_xinput_ui_parent_class)->is_available(ui, error)) {
        return FALSE;
    }

    if (!gpds_xinput_utils_exist_device(gpds_ui_get_device_name(ui))) {
        g_set_error(error,
                    GPDS_XINPUT_UTILS_ERROR,
                    GPDS_XINPUT_UTILS_ERROR_NO_DEVICE,
                    _("No %s found."), 
                    gpds_ui_get_device_name(ui));
        return FALSE;
    }

    return TRUE;
}

static gboolean
dry_run (GpdsUI *ui, GError **error)
{
    GpdsXInputUIPriv *priv;

    priv = GPDS_XINPUT_UI_GET_PRIVATE(ui);
    if (!priv->xinput)
        return FALSE;

    gpds_xinput_backup_all_properties(priv->xinput);

    if (GPDS_UI_CLASS(gpds_xinput_ui_parent_class)->dry_run)
        return GPDS_UI_CLASS(gpds_xinput_ui_parent_class)->dry_run(ui, error);

    return TRUE;
}

static void
finish_dry_run (GpdsUI *ui, GError **error)
{
    GpdsXInputUIPriv *priv;

    priv = GPDS_XINPUT_UI_GET_PRIVATE(ui);
    if (!priv->xinput)
        return;

    gpds_xinput_restore_all_properties(priv->xinput);

    if (GPDS_UI_CLASS(gpds_xinput_ui_parent_class)->finish_dry_run)
        GPDS_UI_CLASS(gpds_xinput_ui_parent_class)->finish_dry_run(ui, error);
}

void
gpds_xinput_ui_set_xinput (GpdsXInputUI *ui, GpdsXInput *xinput)
{
    GpdsXInputUIPriv *priv;
    g_return_if_fail(GPDS_IS_XINPUT_UI(ui));

    priv = GPDS_XINPUT_UI_GET_PRIVATE(ui);
    priv->xinput = xinput;
    if (priv->xinput)
        g_object_ref(priv->xinput);
}

GpdsXInput *
gpds_xinput_ui_get_xinput (GpdsXInputUI *ui)
{
    g_return_val_if_fail(GPDS_IS_XINPUT_UI(ui), NULL);

    return GPDS_XINPUT_UI_GET_PRIVATE(ui)->xinput;
}

static void
show_error (GError *error)
{
    if (!error)
        return;

    g_print("%s\n", error->message);
}

gboolean
gpds_xinput_ui_get_xinput_int_property (GpdsXInputUI *ui,
                                        gint property,
                                        gint **values,
                                        gulong *n_values)
{
    GError *error = NULL;
    GpdsXInputUIPriv *priv;

    g_return_val_if_fail(GPDS_IS_XINPUT_UI(ui), FALSE);

    priv = GPDS_XINPUT_UI_GET_PRIVATE(ui);
    g_return_val_if_fail(priv->xinput, FALSE);

    if (!gpds_xinput_get_int_properties(priv->xinput,
                                        property,
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

gboolean
gpds_xinput_ui_get_xinput_float_property (GpdsXInputUI *ui,
                                          gint property,
                                          gdouble **values,
                                          gulong *n_values)
{
    GError *error = NULL;
    GpdsXInputUIPriv *priv;

    g_return_val_if_fail(GPDS_IS_XINPUT_UI(ui), FALSE);

    priv = GPDS_XINPUT_UI_GET_PRIVATE(ui);
    g_return_val_if_fail(priv->xinput, FALSE);

    if (!gpds_xinput_get_float_properties(priv->xinput,
                                          property,
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

void
gpds_xinput_ui_set_xinput_property_from_toggle_button_state (GpdsXInputUI *ui,
                                                             gint property,
                                                             GtkToggleButton *button)
{
    GError *error = NULL;
    gint properties[1];
    GpdsXInputUIPriv *priv;

    g_return_if_fail(GPDS_IS_XINPUT_UI(ui));
    g_return_if_fail(GTK_TOGGLE_BUTTON(button));

    priv = GPDS_XINPUT_UI_GET_PRIVATE(ui);
    if (!priv->xinput)
        return;

    properties[0] = gtk_toggle_button_get_active(button) ? 1 : 0;

    if (!gpds_xinput_set_int_properties(priv->xinput,
                                        property,
                                        &error,
                                        properties,
                                        1)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

void
gpds_xinput_ui_set_toggle_button_state_from_preference (GpdsXInputUI *ui,
                                                        gint property,
                                                        const gchar *gconf_key_name,
                                                        const gchar *button_name)
{
    GObject *button, *depend_widget;
    gint *values;
    gulong n_values;
    gboolean enable;
    gchar *depend_widget_name;

    g_return_if_fail(GPDS_IS_XINPUT_UI(ui));

    if (!gpds_xinput_ui_get_xinput_int_property(ui,
                                                property,
                                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_bool(GPDS_UI(ui), gconf_key_name, &enable))
        enable = (values[0] == 1);

    button = gpds_ui_get_ui_object_by_name(GPDS_UI(ui), button_name);
    g_return_if_fail(GTK_TOGGLE_BUTTON(button));

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), enable);
    g_free(values);

    depend_widget_name = g_strconcat(button_name, "_box", NULL);
    depend_widget = gpds_ui_get_ui_object_by_name(GPDS_UI(ui), depend_widget_name);
    if (depend_widget)
        gtk_widget_set_sensitive(GTK_WIDGET(depend_widget), enable);
    g_free(depend_widget_name);
}

void
gpds_xinput_ui_set_widget_value_from_preference (GpdsXInputUI *ui,
                                                 gint property,
                                                 const gchar *gconf_key_name,
                                                 const gchar *widget_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gint value;
    gdouble double_value;

    g_return_if_fail(GPDS_IS_XINPUT_UI(ui));

    if (!gpds_xinput_ui_get_xinput_int_property(ui,
                                                property,
                                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_int(GPDS_UI(ui), gconf_key_name, &value))
        value = values[0];

    double_value = value;
    object = gpds_ui_get_ui_object_by_name(GPDS_UI(ui), widget_name);
    if (GTK_IS_RANGE(object))
        object = G_OBJECT(gtk_range_get_adjustment(GTK_RANGE(object)));
    g_object_set(object, "value", double_value, NULL);
    g_free(values);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
