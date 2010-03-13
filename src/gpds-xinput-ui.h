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

#ifndef __GPDS_XINPUT_UI_H__
#define __GPDS_XINPUT_UI_H__

#include <gpds-ui.h>
#include <gpds-xinput.h>

G_BEGIN_DECLS

#define GPDS_XINPUT_UI_ERROR           (gpds_xinput_ui_error_quark())

#define GPDS_TYPE_XINPUT_UI            (gpds_xinput_ui_get_type ())
#define GPDS_XINPUT_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_XINPUT_UI, GpdsXInputUI))
#define GPDS_XINPUT_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_XINPUT_UI, GpdsXInputUIClass))
#define GPDS_IS_XINPUT_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_XINPUT_UI))
#define GPDS_IS_XINPUT_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_XINPUT_UI))
#define GPDS_XINPUT_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_XINPUT_UI, GpdsXInputUIClass))

typedef struct _GpdsXInputUI GpdsXInputUI;
typedef struct _GpdsXInputUIClass GpdsXInputUIClass;

struct _GpdsXInputUI
{
    GpdsUI parent;
};

struct _GpdsXInputUIClass
{
    GpdsUIClass parent_class;
};

GType       gpds_xinput_ui_get_type         (void) G_GNUC_CONST;
void        gpds_xinput_ui_set_xinput       (GpdsXInputUI *ui,
                                             GpdsXInput *xinput);
GpdsXInput *gpds_xinput_ui_get_xinput       (GpdsXInputUI *ui);
gboolean    gpds_xinput_ui_get_xinput_int_property
                                            (GpdsXInputUI *ui,
                                             gint property,
                                             gint **values,
                                             gulong *n_values);
gboolean    gpds_xinput_ui_get_xinput_float_property
                                            (GpdsXInputUI *ui,
                                             gint property,
                                             gdouble **values,
                                             gulong *n_values);
void        gpds_xinput_ui_set_xinput_property_from_toggle_button_state
                                            (GpdsXInputUI *ui,
                                             gint property,
                                             GtkToggleButton *button);
void        gpds_xinput_ui_set_xinput_property_from_range_value
                                            (GpdsXInputUI *ui,
                                             gint property,
                                             GtkRange *range);
void        gpds_xinput_ui_set_toggle_button_state_from_preference
                                            (GpdsXInputUI *ui,
                                             gint property,
                                             const gchar *gconf_key_name,
                                             const gchar *button_name);
void        gpds_xinput_ui_set_gconf_value_from_widget
                                            (GpdsXInputUI *ui,
                                             const gchar *gconf_key_name,
                                             const gchar *widget_name);
void        gpds_xinput_ui_set_widget_value_from_preference
                                            (GpdsXInputUI *ui,
                                             gint property,
                                             const gchar *gconf_key_name,
                                             const gchar *widget_name);

#define GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(function_name, PROPERTY_NAME, depend_widget_name)  \
static void                                                                                             \
cb_ ## function_name ## _toggled (GtkToggleButton *button,                                              \
                                  gpointer user_data)                                                   \
{                                                                                                       \
    GtkBuilder *builder;                                                                                \
    GObject *depend_widget = NULL;                                                                      \
    gboolean enable;                                                                                    \
    gpds_xinput_ui_set_xinput_property_from_toggle_button_state(GPDS_XINPUT_UI(user_data),              \
                                                                PROPERTY_NAME,                          \
                                                                button);                                \
    enable = gtk_toggle_button_get_active(button);                                                      \
    builder = gpds_ui_get_builder(GPDS_UI(user_data));                                                  \
    if (!depend_widget_name)                                                                            \
        return;                                                                                         \
    depend_widget = gtk_builder_get_object(builder, depend_widget_name);                                \
    if (!depend_widget)                                                                                 \
        return;                                                                                         \
    gtk_widget_set_sensitive(GTK_WIDGET(depend_widget), enable);                                        \
}

#define GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(function_name, PROPERTY_NAME)    \
static void                                                                                 \
cb_ ## function_name ## _value_changed (GtkRange *range, gpointer user_data)                \
{                                                                                           \
    gdouble value;                                                                          \
    gpds_xinput_ui_set_xinput_property_from_range_value(GPDS_XINPUT_UI(user_data),          \
                                                        PROPERTY_NAME,                      \
                                                        range);                             \
    value = gtk_range_get_value(range);                                                     \
}

G_END_DECLS

#endif /* __GPDS_XINPUT_UI_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
