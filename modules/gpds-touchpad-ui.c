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
#include "gpds-touchpad-xinput.h"

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
    gpds_touchpad_ui_register_type(type_module);
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
GPDS_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list var_args)
{
    return g_object_new_valist(GPDS_TYPE_TOUCHPAD_UI, first_property, var_args);
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
                        gboolean sensitivity)
{
    GObject *object;

    object = gtk_builder_get_object(builder, widget_id);
    gtk_widget_set_sensitive(GTK_WIDGET(object), sensitivity);
}

static void
set_toggle_property (GpdsXInput *xinput, GtkToggleButton *button, GpdsTouchpadProperty property)
{
    GError *error = NULL;
    gint properties[1];

    properties[0] = gtk_toggle_button_get_active(button) ? 1 : 0;

    if (!gpds_xinput_set_int_properties(xinput,
                                        gpds_touchpad_xinput_get_name(property),
                                        gpds_touchpad_xinput_get_format_type(property),
                                        &error,
                                        properties,
                                        1)) {
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
    GObject *object;
    gint properties[3];

    object = gtk_builder_get_object(builder, "vertical_scrolling");
    properties[0] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)) ? 1 :0;
    set_widget_sensitivity(builder, "vertical_scrolling_box", properties[0]);

    object = gtk_builder_get_object(builder, "horizontal_scrolling");
    properties[1] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)) ? 1 :0;
    set_widget_sensitivity(builder, "horizontal_scrolling_box", properties[1]);

    properties[2] = 0;

    if (!gpds_xinput_set_int_properties(xinput,
                                        gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_EDGE_SCROLLING),
                                        gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_EDGE_SCROLLING),
                                        &error,
                                        properties,
                                        3)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

static void
set_range_property (GpdsXInput *xinput, GtkRange *range, GpdsTouchpadProperty property)
{
    GError *error = NULL;
    gint properties[1];

    properties[0] = (gint)gtk_range_get_value(range);
    if (!gpds_xinput_set_int_properties(xinput,
                                        gpds_touchpad_xinput_get_name(property),
                                        gpds_touchpad_xinput_get_format_type(property),
                                        &error,
                                        properties,
                                        1)) {
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
    GObject *object;
    gint properties[2];

    object = gtk_builder_get_object(builder, "horizontal_scrolling_scale");
    properties[0] = (gint)gtk_range_get_value(GTK_RANGE(object));

    object = gtk_builder_get_object(builder, "vertical_scrolling_scale");
    properties[1] = (gint)gtk_range_get_value(GTK_RANGE(object));

    if (!gpds_xinput_set_int_properties(xinput,
                                        gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_SCROLLING_DISTANCE),
                                        gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_SCROLLING_DISTANCE),
                                        &error,
                                        properties,
                                        2)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

static void
set_circular_scrolling_trigger_property (GpdsTouchpadUI *ui, GpdsTouchpadCircularScrollingTrigger trigger)
{
    GError *error = NULL;
    gint properties[1];

    properties[0] = trigger;

    if (!gpds_xinput_set_int_properties(ui->xinput,
                                        gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER),
                                        gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER),
                                        &error,
                                        properties,
                                        1)) {
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
    gdouble time;

    set_range_property(ui->xinput, range, GPDS_TOUCHPAD_TAP_TIME);

    time = gtk_range_get_value(range);
    gconf_client_set_int(ui->gconf, GPDS_TOUCHPAD_TAP_TIME_KEY, (gint)time, NULL);
}

static void
cb_faster_tapping_check_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    gboolean check;

    set_toggle_property(ui->xinput, button, GPDS_TOUCHPAD_TAP_FAST_TAP);
    check = gtk_toggle_button_get_active(button);
    gconf_client_set_bool(ui->gconf, GPDS_TOUCHPAD_TAP_FAST_TAP_KEY, check, NULL);
}

static void
cb_circular_scrolling_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    GtkBuilder *builder;
    gboolean check;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));

    set_toggle_property(ui->xinput, button, GPDS_TOUCHPAD_CIRCULAR_SCROLLING);

    check = gtk_toggle_button_get_active(button);
    set_widget_sensitivity(builder, "circular_scrolling_box", check);
    gconf_client_set_bool(ui->gconf, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY, check, NULL);
}

static void
cb_vertical_scrolling_toggled (GtkToggleButton *button, gpointer user_data)
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
cb_horizontal_scrolling_toggled (GtkToggleButton *button, gpointer user_data)
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
cb_vertical_scrolling_scale_value_changed (GtkRange *range, gpointer user_data)
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
cb_horizontal_scrolling_scale_value_changed (GtkRange *range, gpointer user_data)
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
set_circular_scrolling_trigger_button_state (GpdsTouchpadUI *ui, 
                                             GpdsTouchpadCircularScrollingTrigger trigger)
{
    GtkToggleButton *button;
    GtkBuilder *builder;
    gboolean active = FALSE;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

#define SET_TOGGLE_BUTTON_STATE(name, state)                           \
    button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, name)); \
    gtk_toggle_button_set_active(button, state);

    if (trigger == GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_ANY)
        active = TRUE;

    SET_TOGGLE_BUTTON_STATE("trigger_top_toggle", active);
    SET_TOGGLE_BUTTON_STATE("trigger_top_right_toggle", active);
    SET_TOGGLE_BUTTON_STATE("trigger_right_toggle", active);
    SET_TOGGLE_BUTTON_STATE("trigger_right_bottom_toggle", active);
    SET_TOGGLE_BUTTON_STATE("trigger_bottom_toggle", active);
    SET_TOGGLE_BUTTON_STATE("trigger_bottom_left_toggle", active);
    SET_TOGGLE_BUTTON_STATE("trigger_left_toggle", active);
    SET_TOGGLE_BUTTON_STATE("trigger_left_top_toggle", active);

    switch (trigger) {
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_TOP:
        SET_TOGGLE_BUTTON_STATE("trigger_top_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_TOP_RIGHT:
        SET_TOGGLE_BUTTON_STATE("trigger_top_right_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_RIGHT:
        SET_TOGGLE_BUTTON_STATE("trigger_right_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_RIGHT_BOTTOM:
        SET_TOGGLE_BUTTON_STATE("trigger_right_bottom_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_BOTTOM:
        SET_TOGGLE_BUTTON_STATE("trigger_bottom_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_BOTTOM_LEFT:
        SET_TOGGLE_BUTTON_STATE("trigger_bottom_left_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_LEFT:
        SET_TOGGLE_BUTTON_STATE("trigger_left_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_LEFT_TOP:
        SET_TOGGLE_BUTTON_STATE("trigger_left_top_toggle", TRUE);
        break;
    case GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_ANY:
    default:
        break;
    }
#undef SET_TOGGLE_BUTTON_STATE
}

#define DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(position, trigger)                                   \
static void                                                                                             \
cb_trigger_ ## position ## _toggle_button_press_event (GtkWidget *widget,                               \
                                                       GdkEventButton *event,                           \
                                                       gpointer user_data)                              \
{                                                                                                       \
    set_circular_scrolling_trigger_property(GPDS_TOUCHPAD_UI(user_data),                                \
                                            GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_ ## trigger);      \
    set_circular_scrolling_trigger_button_state(GPDS_TOUCHPAD_UI(user_data),                            \
                                                GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_ ## trigger);  \
}

DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(top, TOP)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(top_right, TOP_RIGHT)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(right, RIGHT)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(right_bottom, RIGHT_BOTTOM)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(bottom, BOTTOM)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(bottom_left, BOTTOM_LEFT)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(left, LEFT)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(left_top, LEFT_TOP)
DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK(any, ANY)

#undef DEFINE_CIRCULAR_SCROLLING_TRIGGER_CALLBACK

static void
set_sensitivity_depends_on_use_type (GpdsTouchpadUI *ui,
                                     GpdsTouchpadUseType use_type)
{
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

    switch (use_type) {
    case GPDS_TOUCHPAD_USE_TYPE_OFF:
    case GPDS_TOUCHPAD_USE_TYPE_TAPPING_AND_SCROLLING_OFF:
        set_widget_sensitivity(builder, "scrolling_vbox", FALSE);
        set_widget_sensitivity(builder, "tapping_vbox", FALSE);
        break;
    case GPDS_TOUCHPAD_USE_TYPE_NORMAL:
        set_widget_sensitivity(builder, "scrolling_vbox", TRUE);
        set_widget_sensitivity(builder, "tapping_vbox", TRUE);
    default:
        break;
    }
}

static void
set_touchpad_use_type_combo_state (GpdsTouchpadUI *ui, 
                                   GpdsTouchpadUseType use_type)
{
    GtkComboBox *combo;
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

    combo = GTK_COMBO_BOX(gtk_builder_get_object(builder, "touchpad_use_type"));
    gtk_combo_box_set_active(combo, (gint)use_type);
    set_sensitivity_depends_on_use_type(ui, use_type);
}

static void
cb_touchpad_use_type_changed (GtkComboBox *combo, gpointer user_data)
{
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);
    gint properties[1];
    GError *error = NULL;

    properties[0] = gtk_combo_box_get_active(combo);
    if (!gpds_xinput_set_int_properties(ui->xinput,
                                        gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_OFF),
                                        gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_OFF),
                                        &error,
                                        properties,
                                        1)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
    gconf_client_set_bool(ui->gconf, GPDS_TOUCHPAD_OFF_KEY, properties[0], NULL);
    set_sensitivity_depends_on_use_type(ui, properties[0]);
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

    CONNECT(touchpad_use_type, changed);
    CONNECT(tapping_time_scale, value_changed);
    CONNECT(faster_tapping_check, toggled);
    CONNECT(circular_scrolling, toggled);
    CONNECT(vertical_scrolling, toggled);
    CONNECT(vertical_scrolling_scale, value_changed);
    CONNECT(horizontal_scrolling, toggled);
    CONNECT(horizontal_scrolling_scale, value_changed);

    /* cirlular scrolling trigger */
    CONNECT(trigger_top_toggle, button_press_event);
    CONNECT(trigger_top_right_toggle, button_press_event);
    CONNECT(trigger_right_toggle, button_press_event);
    CONNECT(trigger_right_bottom_toggle, button_press_event);
    CONNECT(trigger_bottom_toggle, button_press_event);
    CONNECT(trigger_bottom_left_toggle, button_press_event);
    CONNECT(trigger_left_toggle, button_press_event);
    CONNECT(trigger_left_top_toggle, button_press_event);
    CONNECT(trigger_any_toggle, button_press_event);

#undef CONNECT
}

static gboolean
get_integer_properties (GpdsXInput *xinput, GpdsTouchpadProperty property,
                      gint **values, gulong *n_values)
{
    GError *error = NULL;

    if (!gpds_xinput_get_int_properties(xinput,
                                        gpds_touchpad_xinput_get_name(property),
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
                                      GpdsTouchpadProperty property,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gint value;
    gboolean dir_exists;

    if (!get_integer_properties(ui->xinput, property,
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

static gboolean
set_boolean_property_from_preference (GpdsTouchpadUI *ui,
                                      GpdsTouchpadProperty property,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable, dir_exists;

    if (!get_integer_properties(ui->xinput, property,
                                &values, &n_values)) {
        return FALSE;
    }

    dir_exists = gconf_client_dir_exists(ui->gconf, GPDS_TOUCHPAD_GCONF_DIR, NULL);
    if (dir_exists)
        enable = gconf_client_get_bool(ui->gconf, gconf_key_name, NULL);
    else
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, object_name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    g_free(values);

    return enable;
}

static void
set_edge_scroll_property_from_preference (GpdsTouchpadUI *ui,
                                          GtkBuilder *builder)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable, dir_exists;

    if (!get_integer_properties(ui->xinput, GPDS_TOUCHPAD_EDGE_SCROLLING,
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
    object = gtk_builder_get_object(builder, "vertical_scrolling");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    set_widget_sensitivity(builder, "vertical_scrolling_box", enable);

    if (dir_exists)
        enable = gconf_client_get_bool(ui->gconf, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY, NULL);
    else
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "horizontal_scrolling");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    set_widget_sensitivity(builder, "horizontal_scrolling_box", enable);

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

    if (!get_integer_properties(ui->xinput,
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
    object = gtk_builder_get_object(builder, "vertical_scrolling_scale");
    gtk_range_set_value(GTK_RANGE(object), error ? values[0] : distance);
    if (error)
        g_clear_error(&error);

    distance = gconf_client_get_int(ui->gconf,
                                    GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY,
                                    &error);
    object = gtk_builder_get_object(builder, "horizontal_scrolling_scale");
    gtk_range_set_value(GTK_RANGE(object), error ? values[1] : distance);
    if (error)
        g_clear_error(&error);

    g_free(values);
}

static void
set_circular_scrolling_property_from_preference (GpdsTouchpadUI *ui,
                                                 GtkBuilder *builder)
{
    gboolean enable;

    enable = set_boolean_property_from_preference(ui,
                                                  GPDS_TOUCHPAD_CIRCULAR_SCROLLING,
                                                  GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY,
                                                  builder,
                                                  "circular_scrolling");

    set_widget_sensitivity(builder, "circular_scrolling_box", enable);
}

static void
set_circular_scrolling_trigger_property_from_preference (GpdsTouchpadUI *ui,
                                                         GtkBuilder *builder)
{
    GError *error = NULL;
    gint *values;
    gulong n_values;
    GpdsTouchpadCircularScrollingTrigger trigger;

    if (!get_integer_properties(ui->xinput,
                                GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER,
                                &values, &n_values)) {
        return;
    }

    trigger = gconf_client_get_int(ui->gconf,
                                   GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_KEY,
                                   &error);
    set_circular_scrolling_trigger_button_state(ui, error ? values[0] : trigger);
    if (error)
        g_clear_error(&error);

    g_free(values);
}

static void
set_touchpad_use_type_property_from_preference (GpdsTouchpadUI *ui,
                                                GtkBuilder *builder)
{
    GError *error = NULL;
    gint *values;
    gulong n_values;
    GpdsTouchpadUseType type;

    if (!get_integer_properties(ui->xinput,
                                GPDS_TOUCHPAD_OFF,
                                &values, &n_values)) {
        return;
    }

    type = gconf_client_get_int(ui->gconf,
                                GPDS_TOUCHPAD_OFF_KEY,
                                &error);
    set_touchpad_use_type_combo_state(ui, error ? values[0] : type);
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
    set_circular_scrolling_property_from_preference(touchpad_ui, builder);
    set_edge_scroll_property_from_preference(touchpad_ui, builder);
    set_scroll_distance_property_from_preference(touchpad_ui, builder);
    set_circular_scrolling_trigger_property_from_preference(touchpad_ui, builder);

    set_touchpad_use_type_property_from_preference(touchpad_ui, builder);
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    const gchar *device_name;
    device_name = gpds_touchpad_xinput_find_device_name();

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
