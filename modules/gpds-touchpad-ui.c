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
    GpdsXInputUI parent;
    gchar *ui_file_path;
};

struct _GpdsTouchpadUIClass
{
    GpdsXInputUIClass parent_class;
};

GType gpds_touchpad_ui_get_type (void) G_GNUC_CONST;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static gboolean   dry_run            (GpdsUI  *ui, GError **error);
static void       finish_dry_run     (GpdsUI  *ui, GError **error);
static gboolean   apply              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GdkPixbuf *get_icon_pixbuf    (GpdsUI  *ui, GError **error);
static void       disconnect_signals (GpdsUI  *ui);

G_DEFINE_DYNAMIC_TYPE(GpdsTouchpadUI, gpds_touchpad_ui, GPDS_TYPE_XINPUT_UI)

static void
gpds_touchpad_ui_class_init (GpdsTouchpadUIClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GpdsUIClass *ui_class = GPDS_UI_CLASS(klass);

    gobject_class->dispose = dispose;

    ui_class->is_available       = is_available;
    ui_class->build              = build;
    ui_class->dry_run            = dry_run;
    ui_class->finish_dry_run     = finish_dry_run;
    ui_class->apply              = apply;
    ui_class->get_content_widget = get_content_widget;
    ui_class->get_icon_pixbuf    = get_icon_pixbuf;
}

static void
gpds_touchpad_ui_class_finalize (GpdsTouchpadUIClass *klass)
{
}

static void
gpds_touchpad_ui_init (GpdsTouchpadUI *ui)
{
    ui->ui_file_path = 
        g_build_filename(gpds_get_ui_file_directory(), "touchpad.ui", NULL);
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

    g_free(ui->ui_file_path);
    disconnect_signals(GPDS_UI(ui));

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
set_edge_scrolling_toggle_property (GpdsXInput *xinput, GtkBuilder *builder)
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

    object = gtk_builder_get_object(builder, "continuous_edge_scrolling");
    properties[2] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)) ? 1 :0;

    if (!gpds_xinput_set_int_properties(xinput,
                                        GPDS_TOUCHPAD_EDGE_SCROLLING,
                                        &error,
                                        properties,
                                        3)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(guest_mouse_off,
                                             GPDS_TOUCHPAD_GUEST_MOUSE_OFF,
                                             NULL)
GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(palm_detection,
                                             GPDS_TOUCHPAD_PALM_DETECTION,
                                             "palm_detection_box")
GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(locked_drags,
                                             GPDS_TOUCHPAD_LOCKED_DRAGS,
                                             "locked_drags_box")
GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(faster_tapping_check,
                                             GPDS_TOUCHPAD_TAP_FAST_TAP,
                                             NULL)
GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(circular_scrolling,
                                             GPDS_TOUCHPAD_CIRCULAR_SCROLLING,
                                             "circular_scrolling_box")

GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(locked_drags_timeout_scale,
                                                   GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT)
GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(tapping_time_scale,
                                                   GPDS_TOUCHPAD_TAP_TIME)
GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(tapping_move_scale,
                                                   GPDS_TOUCHPAD_TAP_MOVE)

static void
set_two_finger_scrolling_toggle_property (GpdsXInput *xinput, GtkBuilder *builder)
{
    GError *error = NULL;
    GObject *object;
    gint properties[2];

    object = gtk_builder_get_object(builder, "two_finger_vertical_scrolling");
    properties[0] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)) ? 1 :0;

    object = gtk_builder_get_object(builder, "two_finger_horizontal_scrolling");
    properties[1] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)) ? 1 :0;

    object = gtk_builder_get_object(builder, "continuous_edge_scrolling");
    properties[2] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)) ? 1 :0;

    if (!gpds_xinput_set_int_properties(xinput,
                                        GPDS_TOUCHPAD_TWO_FINGER_SCROLLING,
                                        &error,
                                        properties,
                                        2)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

#define DEFINE_SET_PAIR_SCALE_VALUE(function_name, widget_name1, widget_name2, PROPERTY_NAME)   \
static void                                                                                     \
set_ ## function_name ## _property (GpdsXInput *xinput, GtkBuilder *builder)                    \
{                                                                                               \
    GError *error = NULL;                                                                       \
    GObject *object;                                                                            \
    gint properties[2];                                                                         \
    object = gtk_builder_get_object(builder, widget_name1);                                     \
    properties[0] = (gint)gtk_range_get_value(GTK_RANGE(object));                               \
    object = gtk_builder_get_object(builder, widget_name2);                                     \
    properties[1] = (gint)gtk_range_get_value(GTK_RANGE(object));                               \
    if (!gpds_xinput_set_int_properties(xinput,                                                 \
                                        PROPERTY_NAME,                                          \
                                        &error,                                                 \
                                        properties,                                             \
                                        2)) {                                                   \
        if (error) {                                                                            \
            show_error(error);                                                                  \
            g_error_free(error);                                                                \
        }                                                                                       \
    }                                                                                           \
}

DEFINE_SET_PAIR_SCALE_VALUE(palm_dimensions,
                            "palm_detection_width_scale",
                            "palm_detection_depth_scale",
                            GPDS_TOUCHPAD_PALM_DIMENSIONS)
DEFINE_SET_PAIR_SCALE_VALUE(scrolling_distance,
                            "vertical_scrolling_scale",
                            "horizontal_scrolling_scale",
                            GPDS_TOUCHPAD_SCROLLING_DISTANCE)

static void
set_circular_scrolling_trigger_property (GpdsUI *ui, GpdsTouchpadCircularScrollingTrigger trigger)
{
    GError *error = NULL;
    gint properties[1];
    GpdsXInput *xinput;

    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui));
    if (!xinput)
        return;

    properties[0] = trigger;

    if (!gpds_xinput_set_int_properties(xinput,
                                        GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER,
                                        &error,
                                        properties,
                                        1)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

#define DEFINE_PALM_DIMENSIONS_SCALE_VALUE_CHANGED_CALLBACK(type)                        \
static void                                                                              \
cb_palm_detection_ ## type ## _scale_value_changed (GtkRange *range, gpointer user_data) \
{                                                                                        \
    GtkBuilder *builder;                                                                 \
    GpdsXInput *xinput;                                                                  \
    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(user_data));                       \
    if (!xinput)                                                                         \
        return;                                                                          \
    builder = gpds_ui_get_builder(GPDS_UI(user_data));                                   \
    set_palm_dimensions_property(xinput, builder);                                       \
}

DEFINE_PALM_DIMENSIONS_SCALE_VALUE_CHANGED_CALLBACK(width)
DEFINE_PALM_DIMENSIONS_SCALE_VALUE_CHANGED_CALLBACK(depth)

#define DEFINE_SCROLLING_SCALE_VALUE_CHANGED_CALLBACK(type)                         \
static void                                                                         \
cb_ ## type ## _scrolling_scale_value_changed (GtkRange *range, gpointer user_data) \
{                                                                                   \
    GtkBuilder *builder;                                                            \
    GpdsXInput *xinput;                                                             \
    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(user_data));                  \
    if (!xinput)                                                                    \
        return;                                                                     \
    builder = gpds_ui_get_builder(GPDS_UI(user_data));                              \
    set_scrolling_distance_property(xinput, builder);                               \
}

DEFINE_SCROLLING_SCALE_VALUE_CHANGED_CALLBACK(vertical)
DEFINE_SCROLLING_SCALE_VALUE_CHANGED_CALLBACK(horizontal)

#define DEFINE_EDGE_SCROLLING_CALLBACK(type)                \
static void                                                 \
cb_ ## type ## _scrolling_toggled (GtkToggleButton *button, \
                                   gpointer user_data)      \
{                                                           \
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);       \
    GtkBuilder *builder;                                    \
    GpdsXInput *xinput;                                     \
    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui)); \
    if (!xinput)                                            \
        return;                                             \
    builder = gpds_ui_get_builder(GPDS_UI(user_data));      \
    set_edge_scrolling_toggle_property(xinput, builder);    \
}

DEFINE_EDGE_SCROLLING_CALLBACK(vertical)
DEFINE_EDGE_SCROLLING_CALLBACK(horizontal)
DEFINE_EDGE_SCROLLING_CALLBACK(continuous_edge)

#define DEFINE_TWO_FINGER_SCROLLING_CALLBACK(direction)                     \
static void                                                                 \
cb_two_finger_ ## direction ## _scrolling_toggled (GtkToggleButton *button, \
                                                   gpointer user_data)      \
{                                                                           \
    GpdsTouchpadUI *ui = GPDS_TOUCHPAD_UI(user_data);                       \
    GtkBuilder *builder;                                                    \
    GpdsXInput *xinput;                                                     \
    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui));                 \
    if (!xinput)                                                            \
        return;                                                             \
    builder = gpds_ui_get_builder(GPDS_UI(user_data));                      \
    set_two_finger_scrolling_toggle_property(xinput, builder);              \
}

DEFINE_TWO_FINGER_SCROLLING_CALLBACK(vertical)
DEFINE_TWO_FINGER_SCROLLING_CALLBACK(horizontal)

static GpdsTouchpadCircularScrollingTrigger
get_circular_scrolling_trigger_button_state (GpdsUI *ui)
{
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

#define GET_TOGGLE_BUTTON_STATE(name)                                  \
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, name)))

    if (GET_TOGGLE_BUTTON_STATE("trigger_top_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_TOP;
    else if (GET_TOGGLE_BUTTON_STATE("trigger_top_right_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_TOP_RIGHT;
    else if (GET_TOGGLE_BUTTON_STATE("trigger_right_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_RIGHT;
    else if (GET_TOGGLE_BUTTON_STATE("trigger_right_bottom_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_RIGHT_BOTTOM;
    else if (GET_TOGGLE_BUTTON_STATE("trigger_bottom_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_BOTTOM;
    else if (GET_TOGGLE_BUTTON_STATE("trigger_bottom_left_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_BOTTOM_LEFT;
    else if (GET_TOGGLE_BUTTON_STATE("trigger_left_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_LEFT;
    else if (GET_TOGGLE_BUTTON_STATE("trigger_left_top_toggle"))
        return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_LEFT_TOP;
    return GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_ANY;
#undef GET_TOGGLE_BUTTON_STATE
}

static void
set_circular_scrolling_trigger_button_state (GpdsUI *ui, 
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
    set_circular_scrolling_trigger_property(GPDS_UI(user_data),                                         \
                                            GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_ ## trigger);      \
    set_circular_scrolling_trigger_button_state(GPDS_UI(user_data),                                     \
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
set_sensitivity_depends_on_use_type (GpdsUI *ui,
                                     GpdsTouchpadUseType use_type)
{
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(ui);

    switch (use_type) {
    case GPDS_TOUCHPAD_USE_TYPE_OFF:
        set_widget_sensitivity(builder, "general_box", FALSE);
        set_widget_sensitivity(builder, "scrolling_vbox", FALSE);
        set_widget_sensitivity(builder, "tapping_vbox", FALSE);
        set_widget_sensitivity(builder, "speed_vbox", FALSE);
        break;
    case GPDS_TOUCHPAD_USE_TYPE_TAPPING_AND_SCROLLING_OFF:
        set_widget_sensitivity(builder, "general_box", TRUE);
        set_widget_sensitivity(builder, "scrolling_vbox", FALSE);
        set_widget_sensitivity(builder, "tapping_vbox", FALSE);
        set_widget_sensitivity(builder, "speed_vbox", TRUE);
        break;
    case GPDS_TOUCHPAD_USE_TYPE_NORMAL:
        set_widget_sensitivity(builder, "general_box", TRUE);
        set_widget_sensitivity(builder, "scrolling_vbox", TRUE);
        set_widget_sensitivity(builder, "tapping_vbox", TRUE);
        set_widget_sensitivity(builder, "speed_vbox", TRUE);
    default:
        break;
    }
}

static void
set_touchpad_use_type_property (GpdsUI *ui, GpdsTouchpadUseType use_type)
{
    gint properties[1];
    GError *error = NULL;
    GpdsXInput *xinput;

    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui));
    if (!xinput)
        return;

    properties[0] = use_type;
    if (!gpds_xinput_set_int_properties(xinput,
                                        GPDS_TOUCHPAD_OFF,
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
set_touchpad_use_type (GpdsUI *ui, GpdsTouchpadUseType use_type)
{
    GtkToggleButton *button;
    GtkBuilder *builder;
    gboolean disable_touchpad, disable_tapping_and_scrolling;

    disable_touchpad = (use_type == GPDS_TOUCHPAD_USE_TYPE_OFF);
    disable_tapping_and_scrolling = (use_type == GPDS_TOUCHPAD_USE_TYPE_TAPPING_AND_SCROLLING_OFF);

    builder = gpds_ui_get_builder(ui);
    button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "disable_touchpad"));
    gtk_toggle_button_set_active(button, disable_touchpad);

    button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "disable_tapping_and_scrolling"));
    gtk_toggle_button_set_active(button, disable_tapping_and_scrolling);

    set_sensitivity_depends_on_use_type(ui, use_type);
}

static GpdsTouchpadUseType
get_touchpad_use_type (GpdsUI *ui)
{
    GpdsTouchpadUseType use_type;
    GtkToggleButton *disable_touchpad_button;
    GtkToggleButton *disable_tapping_and_scrolling_button;
    GtkBuilder *builder;
    gboolean disable_touchpad, disable_tapping_and_scrolling;

    builder = gpds_ui_get_builder(ui);
    disable_touchpad_button =
        GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "disable_touchpad"));
    disable_tapping_and_scrolling_button =
        GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "disable_tapping_and_scrolling"));

    disable_touchpad = gtk_toggle_button_get_active(disable_touchpad_button);
    disable_tapping_and_scrolling =
        gtk_toggle_button_get_active(disable_tapping_and_scrolling_button);

    if (disable_touchpad)
        use_type = GPDS_TOUCHPAD_USE_TYPE_OFF;
    else {
        use_type = disable_tapping_and_scrolling ?
            GPDS_TOUCHPAD_USE_TYPE_TAPPING_AND_SCROLLING_OFF :
            GPDS_TOUCHPAD_USE_TYPE_NORMAL;
    }

    return use_type;
}

static void
cb_touchpad_use_type_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsUI *ui = GPDS_UI(user_data);
    GpdsTouchpadUseType use_type;

    use_type = get_touchpad_use_type(ui);
    set_sensitivity_depends_on_use_type(ui, use_type);

    if (gpds_ui_is_dry_run_mode(ui))
        set_touchpad_use_type_property(ui, use_type);
}

static void
cb_disable_touchpad_toggled (GtkToggleButton *button, gpointer user_data)
{
    cb_touchpad_use_type_toggled(button, user_data);
}

static void
cb_disable_tapping_and_scrolling_toggled (GtkToggleButton *button, gpointer user_data)
{
    cb_touchpad_use_type_toggled(button, user_data);
}

static void
cb_disable_while_other_device_exists_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsUI *ui = GPDS_UI(user_data);

    gpds_ui_set_gconf_bool(ui,
                           GPDS_TOUCHPAD_DISABLE_WHILE_OTHER_DEVICE_EXISTS_KEY,
                           gtk_toggle_button_get_active(button));
}

static void
set_tap_time_property (GpdsUI *ui)
{
    GObject *object;
    gint tap_time;
    gboolean disable_tapping;
    gint properties[1];
    GpdsXInput *xinput;
    GError *error = NULL;

    object = gpds_ui_get_ui_object_by_name(ui, "tapping_time_scale");
    tap_time = gtk_range_get_value(GTK_RANGE(object));

    object = gpds_ui_get_ui_object_by_name(ui, "disable_tapping");
    disable_tapping = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object));

    tap_time = disable_tapping ? 0 : tap_time;

    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui));
    if (!xinput)
        return;

    properties[0] = tap_time;
    if (!gpds_xinput_set_int_properties(xinput,
                                        GPDS_TOUCHPAD_TAP_TIME,
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
cb_disable_tapping_toggled (GtkToggleButton *button, gpointer user_data)
{
    GpdsUI *ui = GPDS_UI(user_data);
    gboolean disable_tapping;
    GtkBuilder *builder;

    disable_tapping = gtk_toggle_button_get_active(button);

    if (gpds_ui_is_dry_run_mode(ui))
        set_tap_time_property(ui);

    builder = gpds_ui_get_builder(ui);
    set_widget_sensitivity(builder, "tapping_frame", !disable_tapping);
}

static void
set_move_speed_property (GpdsXInput *xinput, GtkBuilder *builder)
{
    GError *error = NULL;
    GObject *object;
    gdouble properties[4];

    object = gtk_builder_get_object(builder, "minimum_speed_scale");
    properties[0] = gtk_range_get_value(GTK_RANGE(object));

    object = gtk_builder_get_object(builder, "maximum_speed_scale");
    properties[1] = gtk_range_get_value(GTK_RANGE(object));

    object = gtk_builder_get_object(builder, "acceleration_factor_scale");
    properties[2] = gtk_range_get_value(GTK_RANGE(object));

    if (!gpds_xinput_set_float_properties(xinput,
                                          GPDS_TOUCHPAD_MOVE_SPEED,
                                          &error,
                                          properties,
                                          4)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }
}

static void
cb_move_speed_scale_value_changed (GtkRange *range, gpointer user_data)
{
    GtkBuilder *builder;
    GpdsXInput *xinput;

    if (!gpds_ui_is_dry_run_mode(GPDS_UI(user_data)))
        return;

    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(user_data));
    if (!xinput)
        return;

    builder = gpds_ui_get_builder(GPDS_UI(user_data));
    set_move_speed_property(xinput, builder);
}

static void
cb_minimum_speed_scale_value_changed (GtkRange *range, gpointer user_data)
{
    cb_move_speed_scale_value_changed(range, user_data);
}

static void
cb_maximum_speed_scale_value_changed (GtkRange *range, gpointer user_data)
{
    cb_move_speed_scale_value_changed(range, user_data);
}

static void
cb_acceleration_factor_scale_value_changed (GtkRange *range, gpointer user_data)
{
    cb_move_speed_scale_value_changed(range, user_data);
}

static void
connect_signals (GpdsUI *ui)
{
    GObject *object;
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(ui);

#define CONNECT(object_name, signal_name)                               \
    object = gtk_builder_get_object(builder, #object_name);             \
    g_signal_connect(object, #signal_name,                              \
                     G_CALLBACK(cb_ ## object_name ## _ ## signal_name),\
                     ui)

    CONNECT(disable_touchpad, toggled);
    CONNECT(disable_tapping_and_scrolling, toggled);
    CONNECT(disable_while_other_device_exists, toggled);
    CONNECT(guest_mouse_off, toggled);
    CONNECT(palm_detection, toggled);
    CONNECT(palm_detection_width_scale, value_changed);
    CONNECT(palm_detection_depth_scale, value_changed);
    CONNECT(locked_drags, toggled);
    CONNECT(locked_drags_timeout_scale, value_changed);
    CONNECT(disable_tapping, toggled);
    CONNECT(tapping_time_scale, value_changed);
    CONNECT(tapping_move_scale, value_changed);
    CONNECT(faster_tapping_check, toggled);
    CONNECT(circular_scrolling, toggled);
    CONNECT(vertical_scrolling, toggled);
    CONNECT(continuous_edge_scrolling, toggled);
    CONNECT(vertical_scrolling_scale, value_changed);
    CONNECT(horizontal_scrolling, toggled);
    CONNECT(horizontal_scrolling_scale, value_changed);
    CONNECT(two_finger_vertical_scrolling, toggled);
    CONNECT(two_finger_horizontal_scrolling, toggled);

    CONNECT(minimum_speed_scale, value_changed);
    CONNECT(maximum_speed_scale, value_changed);
    CONNECT(acceleration_factor_scale, value_changed);

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

static void
disconnect_signals (GpdsUI *ui)
{
    GObject *object;
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(ui);

#define DISCONNECT(object_name, signal_name)                            \
    object = gtk_builder_get_object(builder, #object_name);             \
    g_signal_handlers_disconnect_by_func(                               \
        object,                                                         \
        G_CALLBACK(cb_ ## object_name ## _ ## signal_name),             \
        ui)

    DISCONNECT(disable_touchpad, toggled);
    DISCONNECT(disable_tapping_and_scrolling, toggled);
    DISCONNECT(disable_while_other_device_exists, toggled);
    DISCONNECT(guest_mouse_off, toggled);
    DISCONNECT(palm_detection, toggled);
    DISCONNECT(palm_detection_width_scale, value_changed);
    DISCONNECT(palm_detection_depth_scale, value_changed);
    DISCONNECT(locked_drags, toggled);
    DISCONNECT(locked_drags_timeout_scale, value_changed);
    DISCONNECT(disable_tapping, toggled);
    DISCONNECT(tapping_time_scale, value_changed);
    DISCONNECT(tapping_move_scale, value_changed);
    DISCONNECT(faster_tapping_check, toggled);
    DISCONNECT(circular_scrolling, toggled);
    DISCONNECT(vertical_scrolling, toggled);
    DISCONNECT(continuous_edge_scrolling, toggled);
    DISCONNECT(vertical_scrolling_scale, value_changed);
    DISCONNECT(horizontal_scrolling, toggled);
    DISCONNECT(horizontal_scrolling_scale, value_changed);
    DISCONNECT(two_finger_vertical_scrolling, toggled);
    DISCONNECT(two_finger_horizontal_scrolling, toggled);

    DISCONNECT(minimum_speed_scale, value_changed);
    DISCONNECT(maximum_speed_scale, value_changed);
    DISCONNECT(acceleration_factor_scale, value_changed);

    /* cirlular scrolling trigger */
    DISCONNECT(trigger_top_toggle, button_press_event);
    DISCONNECT(trigger_top_right_toggle, button_press_event);
    DISCONNECT(trigger_right_toggle, button_press_event);
    DISCONNECT(trigger_right_bottom_toggle, button_press_event);
    DISCONNECT(trigger_bottom_toggle, button_press_event);
    DISCONNECT(trigger_bottom_left_toggle, button_press_event);
    DISCONNECT(trigger_left_toggle, button_press_event);
    DISCONNECT(trigger_left_top_toggle, button_press_event);
    DISCONNECT(trigger_any_toggle, button_press_event);

#undef DISCONNECT
}

static void
set_edge_scrolling_property_from_preference (GpdsUI *ui,
                                             GtkBuilder *builder)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable;

    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                GPDS_TOUCHPAD_EDGE_SCROLLING,
                                                &values, &n_values)) {
        return;
    }

    if (n_values != 3) {
        g_free(values);
        return;
    }

    if (!gpds_ui_get_gconf_bool(ui, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY, &enable))
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "vertical_scrolling");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    set_widget_sensitivity(builder, "vertical_scrolling_box", enable);

    if (!gpds_ui_get_gconf_bool(ui, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY, &enable))
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "horizontal_scrolling");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    set_widget_sensitivity(builder, "horizontal_scrolling_box", enable);

    if (!gpds_ui_get_gconf_bool(ui, GPDS_TOUCHPAD_CONTINUOUS_EDGE_SCROLLING_KEY, &enable))
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "continuous_edge_scrolling");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);

    g_free(values);
}

static void
set_two_finger_scrolling_property_from_preference (GpdsUI *ui,
                                                   GtkBuilder *builder)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable;

    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                GPDS_TOUCHPAD_TWO_FINGER_SCROLLING,
                                                &values, &n_values)) {
        return;
    }

    if (n_values != 2) {
        g_free(values);
        return;
    }

    if (!gpds_ui_get_gconf_bool(ui, GPDS_TOUCHPAD_TWO_FINGER_VERTICAL_SCROLLING_KEY, &enable))
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "two_finger_vertical_scrolling");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);

    if (!gpds_ui_get_gconf_bool(ui, GPDS_TOUCHPAD_TWO_FINGER_HORIZONTAL_SCROLLING_KEY, &enable))
        enable = (values[0] == 1);
    object = gtk_builder_get_object(builder, "two_finger_horizontal_scrolling");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);

    g_free(values);
}

#define DEFINE_SET_INT_PAIR_PROPERTY_FROM_PREFERENCE(function_name, PROPERTY_NAME,                      \
                                                     KEY_NAME1, KEY_NAME2, widget_name1, widget_name2)  \
static void                                                                                             \
set_ ## function_name ## _property_from_preference (GpdsUI *ui, GtkBuilder *builder)                    \
{                                                                                                       \
    GObject *object;                                                                                    \
    gint *values;                                                                                       \
    gulong n_values;                                                                                    \
    gint distance;                                                                                      \
    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),                                     \
                                                PROPERTY_NAME,                                          \
                                                &values, &n_values)) {                                  \
        return;                                                                                         \
    }                                                                                                   \
    if (n_values != 2) {                                                                                \
        g_free(values);                                                                                 \
        return;                                                                                         \
    }                                                                                                   \
    if (!gpds_ui_get_gconf_int(ui, KEY_NAME1, &distance))                                               \
        distance = values[0];                                                                           \
    object = gtk_builder_get_object(builder, widget_name1);                                             \
    gtk_range_set_value(GTK_RANGE(object), distance);                                                   \
    if (!gpds_ui_get_gconf_int(ui, KEY_NAME2, &distance))                                               \
        distance = values[1];                                                                           \
    object = gtk_builder_get_object(builder, widget_name2);                                             \
    gtk_range_set_value(GTK_RANGE(object), distance);                                                   \
    g_free(values);                                                                                     \
}

DEFINE_SET_INT_PAIR_PROPERTY_FROM_PREFERENCE(palm_dimensions,
                                             GPDS_TOUCHPAD_PALM_DIMENSIONS,
                                             GPDS_TOUCHPAD_PALM_DETECTION_WIDTH_KEY,
                                             GPDS_TOUCHPAD_PALM_DETECTION_DEPTH_KEY,
                                             "palm_detection_width_scale",
                                             "palm_detection_depth_scale");
DEFINE_SET_INT_PAIR_PROPERTY_FROM_PREFERENCE(scrolling_distance,
                                             GPDS_TOUCHPAD_SCROLLING_DISTANCE,
                                             GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY,
                                             GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY,
                                             "vertical_scrolling_scale",
                                             "horizontal_scrolling_scale");
static void
set_circular_scrolling_trigger_property_from_preference (GpdsUI *ui)
{
    gint *values;
    gulong n_values;
    GpdsTouchpadCircularScrollingTrigger trigger;

    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER,
                                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_int(ui, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_KEY, (gint*)&trigger))
        trigger = values[0];
    set_circular_scrolling_trigger_button_state(ui, trigger);

    g_free(values);
}

static void
set_touchpad_use_type_property_from_preference (GpdsUI *ui)
{
    gint *values;
    gulong n_values;
    GpdsTouchpadUseType type;

    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                GPDS_TOUCHPAD_OFF,
                                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_int(ui, GPDS_TOUCHPAD_OFF_KEY, (gint*)&type))
        type = values[0];
    set_touchpad_use_type(ui, type);

    g_free(values);
}

static void
set_click_action (GpdsUI *ui)
{
    gint *values;
    gulong n_values;
    gint key;

    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                GPDS_TOUCHPAD_CLICK_ACTION,
                                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_int(ui, GPDS_TOUCHPAD_CLICK_ACTION_FINGER1_KEY, (gint*)&key))
        key = values[0];
    if (!gpds_ui_get_gconf_int(ui, GPDS_TOUCHPAD_CLICK_ACTION_FINGER2_KEY, (gint*)&key))
        key = values[1];
    if (!gpds_ui_get_gconf_int(ui, GPDS_TOUCHPAD_CLICK_ACTION_FINGER3_KEY, (gint*)&key))
        key = values[2];

    g_free(values);
}

static void
set_disable_tapping_from_preference (GpdsUI *ui, GtkBuilder *builder)
{
    gboolean disable_tapping = FALSE;
    GObject *button;

    gpds_ui_get_gconf_bool(ui, GPDS_TOUCHPAD_DISABLE_TAPPING_KEY, &disable_tapping);

    button = gpds_ui_get_ui_object_by_name(ui, "disable_tapping");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), disable_tapping);
    set_widget_sensitivity(builder, "tapping_frame", !disable_tapping);
}

static void
set_move_speed_properties_from_preference (GpdsUI *ui, GtkBuilder *builder)
{
    GObject *object;
    gdouble *values;
    gulong n_values;
    gdouble value;

    if (!gpds_xinput_ui_get_xinput_float_property(GPDS_XINPUT_UI(ui),
                                                  GPDS_TOUCHPAD_MOVE_SPEED,
                                                  &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_float(ui, GPDS_TOUCHPAD_MINIMUM_SPEED_KEY, &value))
        value = values[0];
    object = gtk_builder_get_object(builder, "minimum_speed_scale");
    gtk_range_set_value(GTK_RANGE(object), value);
    if (!gpds_ui_get_gconf_float(ui, GPDS_TOUCHPAD_MAXIMUM_SPEED_KEY, &value))
        value = values[1];
    object = gtk_builder_get_object(builder, "maximum_speed_scale");
    gtk_range_set_value(GTK_RANGE(object), value);
    if (!gpds_ui_get_gconf_float(ui, GPDS_TOUCHPAD_ACCELERATION_FACTOR_KEY, &value))
        value = values[2];
    object = gtk_builder_get_object(builder, "acceleration_factor_scale");
    gtk_range_set_value(GTK_RANGE(object), value);

    g_free(values);
}

static void
set_tapping_time_from_preference (GpdsUI *ui, GtkBuilder *builder)
{
    GObject *object;
    gint *values = NULL;
    gulong n_values;
    gint value;
    gdouble double_value;
    gboolean disable_tapping;

    g_return_if_fail(GPDS_IS_XINPUT_UI(ui));

    if (!gpds_ui_get_gconf_bool(ui, GPDS_TOUCHPAD_DISABLE_TAPPING_KEY, &disable_tapping) ||
        !disable_tapping) {
        if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                    GPDS_TOUCHPAD_TAP_TIME,
                                                    &values, &n_values)) {
            return;
        }
    }

    if (!gpds_ui_get_gconf_int(ui, GPDS_TOUCHPAD_TAP_TIME_KEY, &value) && !values) {
        if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                    GPDS_TOUCHPAD_TAP_TIME,
                                                    &values, &n_values)) {
            return;
        }
        value = values[0];
    }

    double_value = value;
    object = gpds_ui_get_ui_object_by_name(GPDS_UI(ui), "tapping_time_scale");
    if (GTK_IS_RANGE(object))
        object = G_OBJECT(gtk_range_get_adjustment(GTK_RANGE(object)));
    g_object_set(object, "value", double_value, NULL);
    g_free(values);
}

static void
set_gconf_values_to_widget (GpdsUI *ui)
{
    GpdsXInputUI *xinput_ui = GPDS_XINPUT_UI(ui);
    GtkBuilder *builder;

#define SET_INT_VALUE(PROP_NAME, widget_name)                           \
    gpds_xinput_ui_set_widget_value_from_preference(                    \
                                        xinput_ui,                      \
                                        PROP_NAME,                      \
                                        PROP_NAME ## _KEY,              \
                                        widget_name);
#define SET_BOOLEAN_VALUE(PROP_NAME, widget_name)                       \
    gpds_xinput_ui_set_toggle_button_state_from_preference(             \
                                        xinput_ui,                      \
                                        PROP_NAME,                      \
                                        PROP_NAME ## _KEY,              \
                                        widget_name);

    SET_INT_VALUE(GPDS_TOUCHPAD_TAP_MOVE, "tapping_move_scale");
    SET_BOOLEAN_VALUE(GPDS_TOUCHPAD_TAP_FAST_TAP, "faster_tapping_check");
    SET_BOOLEAN_VALUE(GPDS_TOUCHPAD_GUEST_MOUSE_OFF, "guest_mouse_off");
    SET_BOOLEAN_VALUE(GPDS_TOUCHPAD_PALM_DETECTION, "palm_detection");
    SET_BOOLEAN_VALUE(GPDS_TOUCHPAD_LOCKED_DRAGS, "locked_drags");
    SET_INT_VALUE(GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT, "locked_drags_timeout_scale");
    SET_BOOLEAN_VALUE(GPDS_TOUCHPAD_CIRCULAR_SCROLLING, "circular_scrolling");

    builder = gpds_ui_get_builder(ui);

    set_tapping_time_from_preference(ui, builder);
    set_edge_scrolling_property_from_preference(ui, builder);
    set_palm_dimensions_property_from_preference(ui, builder);
    set_scrolling_distance_property_from_preference(ui, builder);
    set_two_finger_scrolling_property_from_preference(ui, builder);
    set_touchpad_use_type_property_from_preference(ui);
    set_circular_scrolling_trigger_property_from_preference(ui);
    set_move_speed_properties_from_preference(ui, builder);
    set_disable_tapping_from_preference(ui, builder);
    set_click_action(ui);
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    if (GPDS_UI_CLASS(gpds_touchpad_ui_parent_class)->is_available &&
        !GPDS_UI_CLASS(gpds_touchpad_ui_parent_class)->is_available(ui, error)) {
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

    return TRUE;
}

static gboolean
build (GpdsUI  *ui, GError **error)
{
    GtkBuilder *builder;
    GpdsXInput *xinput;

    builder = gpds_ui_get_builder(ui);

    if (!gtk_builder_add_from_file(builder, 
                                   GPDS_TOUCHPAD_UI(ui)->ui_file_path,
                                   error)) {
        return FALSE;
    }

    xinput = gpds_touchpad_xinput_new(gpds_ui_get_device_name(ui));
    if (!xinput) {
        return FALSE;
    }
    gpds_xinput_ui_set_xinput(GPDS_XINPUT_UI(ui), xinput);
    g_object_unref(xinput);

    gpds_ui_set_gconf_string(ui, GPDS_GCONF_DEVICE_TYPE_KEY, "touchpad");
    set_gconf_values_to_widget(ui);

    connect_signals(ui);

    return TRUE;
}

static void
set_widget_values_to_xinput (GpdsUI *ui)
{
    GObject *object;
    GtkBuilder *builder;
    GpdsXInput *xinput;
    GpdsTouchpadUseType use_type;
    GpdsTouchpadCircularScrollingTrigger trigger;

    builder = gpds_ui_get_builder(ui);

#define SET_TOGGLE_VALUE(property_name, widget_name)                                       \
    object = gtk_builder_get_object(builder, widget_name);                                 \
    gpds_xinput_ui_set_xinput_property_from_toggle_button_state(GPDS_XINPUT_UI(ui),        \
                                                                property_name,             \
                                                                GTK_TOGGLE_BUTTON(object));
#define SET_RANGE_VALUE(property_name, widget_name)                                \
    object = gtk_builder_get_object(builder, widget_name);                         \
    gpds_xinput_ui_set_xinput_property_from_range_value(GPDS_XINPUT_UI(ui),        \
                                                        property_name,             \
                                                        GTK_RANGE(object));


    SET_RANGE_VALUE(GPDS_TOUCHPAD_TAP_MOVE, "tapping_move_scale");
    SET_TOGGLE_VALUE(GPDS_TOUCHPAD_TAP_FAST_TAP, "faster_tapping_check");
    SET_TOGGLE_VALUE(GPDS_TOUCHPAD_GUEST_MOUSE_OFF, "guest_mouse_off");
    SET_TOGGLE_VALUE(GPDS_TOUCHPAD_PALM_DETECTION, "palm_detection");
    SET_TOGGLE_VALUE(GPDS_TOUCHPAD_LOCKED_DRAGS, "locked_drags");
    SET_RANGE_VALUE(GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT, "locked_drags_timeout_scale");
    SET_TOGGLE_VALUE(GPDS_TOUCHPAD_CIRCULAR_SCROLLING, "circular_scrolling");

    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui));
    set_two_finger_scrolling_toggle_property(xinput, builder);
    set_palm_dimensions_property(xinput, builder);
    set_scrolling_distance_property(xinput, builder);
    set_edge_scrolling_toggle_property(xinput, builder);
    set_move_speed_property(xinput, builder);

    use_type = get_touchpad_use_type(ui);
    set_touchpad_use_type_property(ui, use_type);

    trigger = get_circular_scrolling_trigger_button_state(ui);
    set_circular_scrolling_trigger_property(ui, trigger);

    set_tap_time_property(ui);
}

static void
set_widget_values_to_gconf (GpdsUI *ui)
{
    GpdsTouchpadUseType use_type;
    GpdsTouchpadCircularScrollingTrigger trigger;

#define SET_GCONF_VALUE(gconf_key_name, widget_name)                \
    gpds_xinput_ui_set_gconf_value_from_widget(GPDS_XINPUT_UI(ui),  \
                                               gconf_key_name,      \
                                               widget_name);

    SET_GCONF_VALUE(GPDS_TOUCHPAD_TAP_MOVE_KEY, "tapping_move_scale");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_TAP_TIME_KEY, "tapping_time_scale");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_TAP_FAST_TAP_KEY, "faster_tapping_check");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_GUEST_MOUSE_OFF_KEY, "guest_mouse_off");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_PALM_DETECTION_KEY, "palm_detection");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_LOCKED_DRAGS_KEY, "locked_drags");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT_KEY, "locked_drags_timeout_scale");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY, "circular_scrolling");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_TWO_FINGER_VERTICAL_SCROLLING_KEY, "two_finger_vertical_scrolling");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_TWO_FINGER_HORIZONTAL_SCROLLING_KEY, "two_finger_horizontal_scrolling");

    SET_GCONF_VALUE(GPDS_TOUCHPAD_PALM_DETECTION_WIDTH_KEY, "palm_detection_width_scale");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_PALM_DETECTION_DEPTH_KEY, "palm_detection_depth_scale");

    SET_GCONF_VALUE(GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY, "vertical_scrolling_scale");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY, "horizontal_scrolling_scale");

    SET_GCONF_VALUE(GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY, "vertical_scrolling");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY, "horizontal_scrolling");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_CONTINUOUS_EDGE_SCROLLING_KEY, "conitnuous_edge_scrolling");

    SET_GCONF_VALUE(GPDS_TOUCHPAD_MINIMUM_SPEED_KEY, "minimum_speed_scale");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_MAXIMUM_SPEED_KEY, "maximum_speed_scale");
    SET_GCONF_VALUE(GPDS_TOUCHPAD_ACCELERATION_FACTOR_KEY, "acceleration_factor_scale");

    SET_GCONF_VALUE(GPDS_TOUCHPAD_DISABLE_TAPPING_KEY, "disable_tapping");

    use_type = get_touchpad_use_type(ui);
    gpds_ui_set_gconf_int(ui, GPDS_TOUCHPAD_OFF_KEY, (gint)use_type);

    trigger = get_circular_scrolling_trigger_button_state(ui);
    gpds_ui_set_gconf_int(ui, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_KEY, (gint)trigger);
}

static gboolean
dry_run (GpdsUI *ui, GError **error)
{
    gboolean ret;

    if (GPDS_UI_CLASS(gpds_touchpad_ui_parent_class)->dry_run)
        ret = GPDS_UI_CLASS(gpds_touchpad_ui_parent_class)->dry_run(ui, error);

    set_widget_values_to_xinput(ui);

    return TRUE;
}

static void
finish_dry_run(GpdsUI *ui, GError **error)
{
    set_gconf_values_to_widget(ui);

    if (GPDS_UI_CLASS(gpds_touchpad_ui_parent_class)->finish_dry_run)
        GPDS_UI_CLASS(gpds_touchpad_ui_parent_class)->finish_dry_run(ui, error);
}

static gboolean
apply (GpdsUI *ui, GError **error)
{
    set_widget_values_to_xinput(ui);
    set_widget_values_to_gconf(ui);

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
        return NULL;
    }

    return GTK_WIDGET(widget);
}

static GdkPixbuf *
get_icon_pixbuf (GpdsUI *ui, GError **error)
{
    gchar *path;
    GdkPixbuf *pixbuf;

    path = g_build_filename(gpds_get_icon_file_directory(),
                            "touchpad.png", NULL);
    pixbuf = gdk_pixbuf_new_from_file(path, error);
    g_free(path);

    return pixbuf;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
