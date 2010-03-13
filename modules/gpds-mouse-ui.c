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
#include <gpds-xinput-ui.h>
#include <gpds-xinput-utils.h>
#include <gconf/gconf-client.h>

#include "gpds-mouse-definitions.h"
#include "gpds-mouse-xinput.h"

#define GPDS_TYPE_MOUSE_UI            (gpds_mouse_ui_get_type())
#define GPDS_MOUSE_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_MOUSE_UI, GpdsMouseUI))
#define GPDS_MOUSE_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_MOUSE_UI, GpdsMouseUIClass))
#define G_IS_MOUSE_UI(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_MOUSE_UI))
#define G_IS_MOUSE_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_MOUSE_UI))
#define GPDS_MOUSE_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_MOUSE_UI, GpdsMouseUIClass))

typedef struct _GpdsMouseUI GpdsMouseUI;
typedef struct _GpdsMouseUIClass GpdsMouseUIClass;

struct _GpdsMouseUI
{
    GpdsXInputUI parent;
    gchar *ui_file_path;
};

struct _GpdsMouseUIClass
{
    GpdsXInputUIClass parent_class;
};

GType gpds_mouse_ui_get_type (void) G_GNUC_CONST;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static gboolean   dry_run            (GpdsUI  *ui, GError **error);
static void       finish_dry_run     (GpdsUI  *ui, GError **error);
static gboolean   apply              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GdkPixbuf *get_icon_pixbuf    (GpdsUI  *ui, GError **error);

G_DEFINE_DYNAMIC_TYPE(GpdsMouseUI, gpds_mouse_ui, GPDS_TYPE_XINPUT_UI)

static void
gpds_mouse_ui_class_init (GpdsMouseUIClass *klass)
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
gpds_mouse_ui_class_finalize (GpdsMouseUIClass *klass)
{
}

static void
gpds_mouse_ui_init (GpdsMouseUI *ui)
{
    ui->ui_file_path = g_build_filename(gpds_get_ui_file_directory(),
                                        "mouse.ui",
                                        NULL);
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    gpds_mouse_ui_register_type(type_module);
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
GPDS_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list var_args)
{
    return g_object_new_valist(GPDS_TYPE_MOUSE_UI, first_property, var_args);
}

static void
dispose (GObject *object)
{
    GpdsMouseUI *ui = GPDS_MOUSE_UI(object);

    g_free(ui->ui_file_path);

    if (G_OBJECT_CLASS(gpds_mouse_ui_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_mouse_ui_parent_class)->dispose(object);
}

static void
show_error (GError *error)
{
    if (!error)
        return;

    g_print("%s\n", error->message);
}

GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(middle_button_emulation,
                                             GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                             "middle_button_emulation_box")
GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(wheel_emulation,
                                             GPDS_MOUSE_WHEEL_EMULATION,
                                             "wheel_emulation_box")

GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(wheel_emulation_inertia_scale,
                                                   GPDS_MOUSE_WHEEL_EMULATION_INERTIA)
GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(wheel_emulation_timeout_scale,
                                                   GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT)
GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(middle_button_timeout_scale,
                                                   GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT)

static gint
get_wheel_emulation_button (GpdsMouseUI *ui)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GObject *combo;
    GValue value = {0};
    GtkBuilder *builder;
    gint button;

    builder = gpds_ui_get_builder(GPDS_UI(ui));
    combo = gtk_builder_get_object(builder, "wheel_emulation_button");

    if (!GTK_IS_COMBO_BOX(combo))
        return -1;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo), &iter))
        return -1;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));

    gtk_tree_model_get_value(model,
                             &iter,
                             0,
                             &value);
    button = g_value_get_int(&value);
    g_value_unset(&value);

    return button;
}

static void
set_wheel_emulation_button_to_xinput (GpdsMouseUI *ui)
{
    gint button;
    gint properties[1];
    GError *error = NULL;
    GpdsXInput *xinput;

    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui));
    if (!xinput)
        return;

    button = get_wheel_emulation_button(ui);

    properties[0] = button;
    if (!gpds_xinput_set_int_properties(xinput,
                                        GPDS_MOUSE_WHEEL_EMULATION_BUTTON,
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
set_wheel_emulation_button_to_gconf (GpdsMouseUI *ui)
{
    gint button;

    button = get_wheel_emulation_button(ui);

    if (button < 0) {
        gpds_ui_set_gconf_int(GPDS_UI(ui),
                              GPDS_MOUSE_WHEEL_EMULATION_BUTTON_KEY,
                              button);
    }
}

static void
cb_wheel_emulation_button_changed (GtkComboBox *combo, gpointer user_data)
{
    set_wheel_emulation_button_to_xinput(GPDS_MOUSE_UI(user_data));
}

static void
set_scroll_axes_property (GpdsMouseUI *ui)
{
    GtkBuilder *builder;
    GtkToggleButton *button;
    GError *error = NULL;
    gboolean active;
    gint properties[4];
    GpdsXInput *xinput;

    xinput = gpds_xinput_ui_get_xinput(GPDS_XINPUT_UI(ui));
    if (!xinput)
        return;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

    button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wheel_emulation_horizontal"));
    active = gtk_toggle_button_get_active(button);
    if (active) {
        properties[0] = 6;
        properties[1] = 7;
    } else {
        properties[0] = 0;
        properties[1] = 0;
    }

    button = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "wheel_emulation_vertical"));
    active = gtk_toggle_button_get_active(button);
    if (active) {
        properties[2] = 4;
        properties[3] = 5;
    } else {
        properties[2] = 0;
        properties[3] = 0;
    }

    gpds_xinput_set_int_properties(xinput,
                                   GPDS_MOUSE_WHEEL_EMULATION_AXES,
                                   &error,
                                   properties,
                                   4);
    if (error) {
        show_error(error);
        g_error_free(error);
    }
}

#define DEFINE_WHEEL_EMULATION_SCROLL_BUTTON_TOGGLED_CALLBACK(name, KEY_NAME)       \
static void                                                                         \
cb_ ## name ## _toggled (GtkToggleButton *button,                                   \
                         gpointer user_data)                                        \
{                                                                                   \
    gboolean enable;                                                                \
    GpdsMouseUI *ui = GPDS_MOUSE_UI(user_data);                                     \
    set_scroll_axes_property(ui);                                                   \
    enable = gtk_toggle_button_get_active(button);                                  \
}

DEFINE_WHEEL_EMULATION_SCROLL_BUTTON_TOGGLED_CALLBACK(wheel_emulation_vertical, WHEEL_EMULATION_Y_AXIS)
DEFINE_WHEEL_EMULATION_SCROLL_BUTTON_TOGGLED_CALLBACK(wheel_emulation_horizontal, WHEEL_EMULATION_X_AXIS)

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

    CONNECT(middle_button_emulation, toggled);
    CONNECT(middle_button_timeout_scale, value_changed);
    CONNECT(wheel_emulation, toggled);
    CONNECT(wheel_emulation_timeout_scale, value_changed);
    CONNECT(wheel_emulation_button, changed);
    CONNECT(wheel_emulation_inertia_scale, value_changed);
    CONNECT(wheel_emulation_vertical, toggled);
    CONNECT(wheel_emulation_horizontal, toggled);

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

    DISCONNECT(middle_button_emulation, toggled);
    DISCONNECT(middle_button_timeout_scale, value_changed);
    DISCONNECT(wheel_emulation, toggled);
    DISCONNECT(wheel_emulation_timeout_scale, value_changed);
    DISCONNECT(wheel_emulation_button, changed);
    DISCONNECT(wheel_emulation_inertia_scale, value_changed);
    DISCONNECT(wheel_emulation_vertical, toggled);
    DISCONNECT(wheel_emulation_horizontal, toggled);

#undef DISCONNECT
}

static void
set_scroll_axes_property_from_preference (GpdsUI *ui)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean horizontal_enable = FALSE, vertical_enable = FALSE;
    GtkBuilder *builder;

    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                GPDS_MOUSE_WHEEL_EMULATION_AXES,
                                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_bool(ui, GPDS_MOUSE_WHEEL_EMULATION_X_AXIS_KEY, &horizontal_enable))
        if (n_values >= 2)
            horizontal_enable = (values[0] != 0 && values[1] != 0);
    if (!gpds_ui_get_gconf_bool(ui, GPDS_MOUSE_WHEEL_EMULATION_Y_AXIS_KEY, &vertical_enable))
        if (n_values >= 4)
            vertical_enable = (values[2] != 0 && values[3] != 0);

    builder = gpds_ui_get_builder(ui);
    object = gtk_builder_get_object(builder, "wheel_emulation_horizontal");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), horizontal_enable);
    object = gtk_builder_get_object(builder, "wheel_emulation_vertical");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), vertical_enable);

    g_free(values);
}

static void
setup_num_buttons (GpdsUI *ui)
{
    GObject *list_store;
    gshort num_buttons, i;
    GError *error = NULL;
    GtkBuilder *builder;

    builder = gpds_ui_get_builder(ui);

    num_buttons = gpds_xinput_utils_get_device_num_buttons(gpds_ui_get_device_name(ui),
                                                           &error);
    if (error) {
        show_error(error);
        g_error_free(error);
        return;
    }

    list_store = gtk_builder_get_object(builder, "wheel_emulation_button_list_store");

    for (i = num_buttons -1; i > 0; i--) {
        GtkTreeIter iter;
        gtk_list_store_prepend(GTK_LIST_STORE(list_store), &iter);
        gtk_list_store_set(GTK_LIST_STORE(list_store), &iter, 0, i, -1);
    }
}

static gboolean
each_tree_model_iter (GtkTreeModel *model,
                      GtkTreePath *path,
                      GtkTreeIter *iter,
                      gpointer data)
{
    GValue value = {0};
    gint *list_index = data;
    gint int_value;
    gtk_tree_model_get_value(model,
                             iter,
                             0,
                             &value);
    int_value = g_value_get_int(&value);
    g_value_unset(&value);

    if (int_value == *list_index)  {
        gint *indices;
        indices = gtk_tree_path_get_indices(path);
        if (indices)
            *list_index = indices[0];
        return TRUE;
    }
    return FALSE;
}

static void
set_wheel_emulation_button_combo_state (GpdsUI *ui, gint button)
{
    GObject *list_store;
    GtkComboBox *combo;
    GtkBuilder *builder;
    gint list_index = button;

    builder = gpds_ui_get_builder(ui);

    list_store = gtk_builder_get_object(builder, "wheel_emulation_button_list_store");
    gtk_tree_model_foreach(GTK_TREE_MODEL(list_store),
                           each_tree_model_iter, &list_index);

    combo = GTK_COMBO_BOX(gtk_builder_get_object(builder, "wheel_emulation_button"));
    gtk_combo_box_set_active(combo, list_index);
}

static void
set_wheel_emulation_button_property_from_preference (GpdsUI *ui)
{
    gint *values;
    gulong n_values;
    gint button;

    if (!gpds_xinput_ui_get_xinput_int_property(GPDS_XINPUT_UI(ui),
                                                GPDS_MOUSE_WHEEL_EMULATION_BUTTON,
                                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_int(GPDS_UI(ui), GPDS_MOUSE_WHEEL_EMULATION_BUTTON_KEY, &button))
        button = values[0];
    set_wheel_emulation_button_combo_state(ui, button);

    g_free(values);
}

static void
set_gconf_values_to_widget (GpdsUI *ui)
{
    GpdsXInputUI *xinput_ui = GPDS_XINPUT_UI(ui);

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

    SET_BOOLEAN_VALUE(GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                      "middle_button_emulation");
    SET_BOOLEAN_VALUE(GPDS_MOUSE_WHEEL_EMULATION,
                      "wheel_emulation");

    SET_INT_VALUE(GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT,
                  "middle_button_timeout_scale");
    SET_INT_VALUE(GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT,
                  "wheel_emulation_timeout_scale");
    SET_INT_VALUE(GPDS_MOUSE_WHEEL_EMULATION_INERTIA,
                  "wheel_emulation_inertia_scale");

    setup_num_buttons(ui);
    set_wheel_emulation_button_property_from_preference(ui);
    set_scroll_axes_property_from_preference(ui);
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    if (GPDS_UI_CLASS(gpds_mouse_ui_parent_class)->is_available &&
        !GPDS_UI_CLASS(gpds_mouse_ui_parent_class)->is_available(ui, error)) {
        return FALSE;
    }

    if (!g_file_test(GPDS_MOUSE_UI(ui)->ui_file_path,
                     G_FILE_TEST_EXISTS)) {
        g_set_error(error,
                    GPDS_UI_ERROR,
                    GPDS_UI_ERROR_NO_UI_FILE,
                    _("%s did not find."),
                    GPDS_MOUSE_UI(ui)->ui_file_path);
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
                                   GPDS_MOUSE_UI(ui)->ui_file_path,
                                   error)) {
        return FALSE;
    }

    xinput = gpds_mouse_xinput_new(gpds_ui_get_device_name(ui));
    if (!xinput) {
        return FALSE;
    }
    gpds_xinput_ui_set_xinput(GPDS_XINPUT_UI(ui), xinput);
    g_object_unref(xinput);

    gpds_ui_set_gconf_string(ui, GPDS_GCONF_DEVICE_TYPE_KEY, "mouse");
    set_gconf_values_to_widget(ui);

    return TRUE;
}

static void
set_widget_values_to_xinput (GpdsUI *ui)
{
    GObject *object;
    GtkBuilder *builder;

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

    SET_TOGGLE_VALUE(GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                     "middle_button_emulation");
    SET_TOGGLE_VALUE(GPDS_MOUSE_WHEEL_EMULATION,
                     "wheel_emulation");

    SET_RANGE_VALUE(GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT,
                    "middle_button_timeout_scale");
    SET_RANGE_VALUE(GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT,
                    "wheel_emulation_timeout_scale");
    SET_RANGE_VALUE(GPDS_MOUSE_WHEEL_EMULATION_INERTIA,
                    "wheel_emulation_inertia_scale");

    set_wheel_emulation_button_to_xinput(GPDS_MOUSE_UI(ui));
    set_scroll_axes_property(GPDS_MOUSE_UI(ui));
}

static void
set_widget_values_to_gconf (GpdsUI *ui)
{
#define SET_GCONF_VALUE(gconf_key_name, widget_name)                \
    gpds_xinput_ui_set_gconf_value_from_widget(GPDS_XINPUT_UI(ui),  \
                                               gconf_key_name,      \
                                               widget_name);

    SET_GCONF_VALUE(GPDS_MOUSE_MIDDLE_BUTTON_EMULATION_KEY,
                     "middle_button_emulation");
    SET_GCONF_VALUE(GPDS_MOUSE_WHEEL_EMULATION_KEY,
                     "wheel_emulation");
    SET_GCONF_VALUE(GPDS_MOUSE_WHEEL_EMULATION_X_AXIS_KEY,
                     "wheel_emulation_horizontal");
    SET_GCONF_VALUE(GPDS_MOUSE_WHEEL_EMULATION_Y_AXIS_KEY,
                     "wheel_emulation_vertical");

    SET_GCONF_VALUE(GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT_KEY,
                    "middle_button_timeout_scale");
    SET_GCONF_VALUE(GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT_KEY,
                    "wheel_emulation_timeout_scale");
    SET_GCONF_VALUE(GPDS_MOUSE_WHEEL_EMULATION_INERTIA_KEY,
                    "wheel_emulation_inertia_scale");

    set_wheel_emulation_button_to_gconf(GPDS_MOUSE_UI(ui));
}

static gboolean
dry_run (GpdsUI *ui, GError **error)
{
    gboolean ret;

    if (GPDS_UI_CLASS(gpds_mouse_ui_parent_class)->dry_run)
        ret = GPDS_UI_CLASS(gpds_mouse_ui_parent_class)->dry_run(ui, error);

    connect_signals(ui);

    set_widget_values_to_xinput(ui);

    return TRUE;
}

static void
finish_dry_run(GpdsUI *ui, GError **error)
{
    disconnect_signals(ui);
    set_gconf_values_to_widget(ui);

    if (GPDS_UI_CLASS(gpds_mouse_ui_parent_class)->finish_dry_run)
        GPDS_UI_CLASS(gpds_mouse_ui_parent_class)->finish_dry_run(ui, error);
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
                            "mouse.png", NULL);
    pixbuf = gdk_pixbuf_new_from_file(path, error);
    g_free(path);

    return pixbuf;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
