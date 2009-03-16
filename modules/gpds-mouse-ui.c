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
#include <gpds-xinput-utils.h>
#include <gconf/gconf-client.h>

#include "gpds-mouse-definitions.h"
#include "gpds-mouse-xinput.h"

#define GPDS_TYPE_MOUSE_UI            (gpds_mouse_ui_get_type())
#define GPDS_MOUSE_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_MOUSE_UI, GpdsMouseUI))
#define GPDS_MOUSE_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_MOUSE_UI, GpdsMouseUIClass))
#define G_IS_MOUSE_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_MOUSE_UI))
#define G_IS_MOUSE_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_MOUSE_UI))
#define GPDS_MOUSE_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_MOUSE_UI, GpdsMouseUIClass))

typedef struct _GpdsMouseUI GpdsMouseUI;
typedef struct _GpdsMouseUIClass GpdsMouseUIClass;

struct _GpdsMouseUI
{
    GpdsUI parent;
    GpdsXInput *xinput;
    gchar *ui_file_path;
};

struct _GpdsMouseUIClass
{
    GpdsUIClass parent_class;
};

GType gpds_mouse_ui_get_type (void) G_GNUC_CONST;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GdkPixbuf *get_icon_pixbuf    (GpdsUI  *ui, GError **error);

G_DEFINE_DYNAMIC_TYPE(GpdsMouseUI, gpds_mouse_ui, GPDS_TYPE_UI)

static void
gpds_mouse_ui_class_init (GpdsMouseUIClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GpdsUIClass *ui_class = GPDS_UI_CLASS(klass);

    gobject_class->dispose = dispose;

    ui_class->is_available       = is_available;
    ui_class->build              = build;
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
    ui->xinput = NULL;
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

    if (ui->xinput) {
        g_object_unref(ui->xinput);
        ui->xinput = NULL;
    }

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

static void
set_toggle_property (GpdsXInput *xinput, GtkToggleButton *button, GpdsMouseProperty property)
{
    GError *error = NULL;
    gboolean active;
    gint properties[1];

    active = gtk_toggle_button_get_active(button);

    properties[0] = active ? 1 : 0;
    gpds_xinput_set_int_properties(xinput,
                                   property,
                                   &error,
                                   properties,
                                   1);
    if (error) {
        show_error(error);
        g_error_free(error);
    }
}

static void
set_spin_property (GpdsXInput *xinput, GtkSpinButton *button, GpdsMouseProperty property)
{
    GError *error = NULL;
    gdouble value;
    gint properties[1];

    value = gtk_spin_button_get_value(button);

    properties[0] = (gint)value;
    gpds_xinput_set_int_properties(xinput,
                                   property,
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

#define DEFINE_EMULATION_BUTTON_TOGGLED_CALLBACK(name, KEY_NAME, box_name)          \
static void                                                                         \
cb_ ## name ## _toggled (GtkToggleButton *button,                                   \
                         gpointer user_data)                                        \
{                                                                                   \
    GtkBuilder *builder;                                                            \
    gboolean enable;                                                                \
    GpdsMouseUI *ui = GPDS_MOUSE_UI(user_data);                                     \
    set_toggle_property(ui->xinput, button, GPDS_MOUSE_ ## KEY_NAME);               \
    enable = gtk_toggle_button_get_active(button);                                  \
    gpds_ui_set_gconf_bool(GPDS_UI(ui), GPDS_MOUSE_ ## KEY_NAME ## _KEY, enable);   \
    builder = gpds_ui_get_builder(GPDS_UI(user_data));                              \
    set_widget_sensitivity(builder, box_name, button);                              \
}

DEFINE_EMULATION_BUTTON_TOGGLED_CALLBACK(middle_button_emulation, MIDDLE_BUTTON_EMULATION, "middle_button_emulation_box")
DEFINE_EMULATION_BUTTON_TOGGLED_CALLBACK(wheel_emulation, WHEEL_EMULATION, "wheel_emulation_box")

static void
cb_wheel_emulation_button_changed (GtkComboBox *combo, gpointer user_data)
{
    gint properties[1];
    GtkTreeIter iter;
    GObject *list_store;
    GValue value = {0};
    GError *error = NULL;
    GpdsMouseUI *ui = GPDS_MOUSE_UI(user_data);
    GtkBuilder *builder;

    if (!gtk_combo_box_get_active_iter(combo, &iter))
        return;

    builder = gpds_ui_get_builder(GPDS_UI(ui));
    list_store = gtk_builder_get_object(builder, "wheel_emulation_button_list_store");
    gtk_tree_model_get_value(GTK_TREE_MODEL(list_store),
                             &iter,
                             0,
                             &value);
    properties[0] = g_value_get_int(&value);
    g_value_unset(&value);

    if (!gpds_xinput_set_int_properties(ui->xinput,
                                        GPDS_MOUSE_WHEEL_EMULATION_BUTTON,
                                        &error,
                                        properties,
                                        1)) {
        if (error) {
            show_error(error);
            g_error_free(error);
        }
    }

    gpds_ui_set_gconf_int(GPDS_UI(ui), GPDS_MOUSE_WHEEL_EMULATION_BUTTON_KEY, properties[0]);
}

static void
set_scroll_axes_property (GpdsMouseUI *ui)
{
    GtkBuilder *builder;
    GtkToggleButton *button;
    GError *error = NULL;
    gboolean active;
    gint properties[4];

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

    gpds_xinput_set_int_properties(ui->xinput,
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
    gpds_ui_set_gconf_bool(GPDS_UI(ui), GPDS_MOUSE_ ## KEY_NAME ## _KEY, enable);   \
}

DEFINE_WHEEL_EMULATION_SCROLL_BUTTON_TOGGLED_CALLBACK(wheel_emulation_vertical, WHEEL_EMULATION_Y_AXIS)
DEFINE_WHEEL_EMULATION_SCROLL_BUTTON_TOGGLED_CALLBACK(wheel_emulation_horizontal, WHEEL_EMULATION_X_AXIS)

#define DEFINE_SPIN_BUTTON_VALUE_CHANGED_CALLBACK(name, NAME)                       \
static void                                                                         \
cb_ ## name ## _value_changed (GtkSpinButton *button,                               \
                               gpointer user_data)                                  \
{                                                                                   \
    gdouble value;                                                                  \
    GpdsMouseUI *ui = GPDS_MOUSE_UI(user_data);                                     \
    set_spin_property(ui->xinput, button, GPDS_MOUSE_ ## NAME);                     \
    value = gtk_spin_button_get_value(button);                                      \
    gpds_ui_set_gconf_int(GPDS_UI(ui), GPDS_MOUSE_ ## NAME ## _KEY, (gint)value);   \
}

DEFINE_SPIN_BUTTON_VALUE_CHANGED_CALLBACK(wheel_emulation_timeout, WHEEL_EMULATION_TIMEOUT)
DEFINE_SPIN_BUTTON_VALUE_CHANGED_CALLBACK(wheel_emulation_inertia, WHEEL_EMULATION_INERTIA)
DEFINE_SPIN_BUTTON_VALUE_CHANGED_CALLBACK(middle_button_timeout, MIDDLE_BUTTON_TIMEOUT)

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
    CONNECT(wheel_emulation_button, changed);
    CONNECT(wheel_emulation_inertia, value_changed);
    CONNECT(wheel_emulation_vertical, toggled);
    CONNECT(wheel_emulation_horizontal, toggled);

#undef CONNECT
}

static gboolean
get_integer_properties (GpdsXInput *xinput, gint property_enum,
                        gint **values, gulong *n_values)
{
    GError *error = NULL;

    if (!gpds_xinput_get_int_properties(xinput,
                                        property_enum,
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
set_integer_property_from_preference (GpdsMouseUI *ui,
                                      GpdsMouseProperty property,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gint value;

    if (!get_integer_properties(ui->xinput, property,
                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_int(GPDS_UI(ui), gconf_key_name, &value))
        value = values[0];

    object = gtk_builder_get_object(builder, object_name);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(object), value);
    g_free(values);
}

static void
set_boolean_property_from_preference (GpdsMouseUI *ui,
                                      GpdsMouseProperty property,
                                      const gchar *gconf_key_name,
                                      GtkBuilder *builder,
                                      const gchar *object_name)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean enable;
    gchar *box_name;

    if (!get_integer_properties(ui->xinput, property,
                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_bool(GPDS_UI(ui), gconf_key_name, &enable))
        enable = (values[0] == 1);

    object = gtk_builder_get_object(builder, object_name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(object), enable);
    g_free(values);

    box_name = g_strconcat(object_name, "_box", NULL);
    set_widget_sensitivity (builder, box_name, GTK_TOGGLE_BUTTON(object));
    g_free(box_name);
}

static void
set_scroll_axes_property_from_preference (GpdsMouseUI *ui,
                                          GtkBuilder *builder)
{
    GObject *object;
    gint *values;
    gulong n_values;
    gboolean horizontal_enable = FALSE, vertical_enable = FALSE;

    if (!get_integer_properties(ui->xinput, 
                                GPDS_MOUSE_WHEEL_EMULATION_AXES,
                                &values, &n_values)) {
        return;
    }

    if (!gpds_ui_get_gconf_bool(GPDS_UI(ui), GPDS_MOUSE_WHEEL_EMULATION_X_AXIS_KEY, &horizontal_enable))
        if (n_values >= 2)
            horizontal_enable = (values[0] != 0 && values[1] != 0);
    if (!gpds_ui_get_gconf_bool(GPDS_UI(ui), GPDS_MOUSE_WHEEL_EMULATION_Y_AXIS_KEY, &vertical_enable))
        if (n_values >= 4)
            vertical_enable = (values[2] != 0 && values[3] != 0);

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
set_wheel_emulation_button_combo_state (GpdsMouseUI *ui, gint button)
{
    GObject *list_store;
    GtkComboBox *combo;
    GtkBuilder *builder;
    gint list_index = button;

    builder = gpds_ui_get_builder(GPDS_UI(ui));

    list_store = gtk_builder_get_object(builder, "wheel_emulation_button_list_store");
    gtk_tree_model_foreach(GTK_TREE_MODEL(list_store),
                           each_tree_model_iter, &list_index);

    combo = GTK_COMBO_BOX(gtk_builder_get_object(builder, "wheel_emulation_button"));
    gtk_combo_box_set_active(combo, list_index);
}

static void
set_wheel_emulation_button_property_from_preference (GpdsMouseUI *ui)
{
    gint *values;
    gulong n_values;
    gint button;

    if (!get_integer_properties(ui->xinput,
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
setup_current_values (GpdsUI *ui, GtkBuilder *builder)
{
    GpdsMouseUI *mouse_ui = GPDS_MOUSE_UI(ui);

    set_boolean_property_from_preference(mouse_ui,
                                         GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
                                         GPDS_MOUSE_MIDDLE_BUTTON_EMULATION_KEY,
                                         builder,
                                         "middle_button_emulation");
    set_boolean_property_from_preference(mouse_ui,
                                         GPDS_MOUSE_WHEEL_EMULATION,
                                         GPDS_MOUSE_WHEEL_EMULATION_KEY,
                                         builder,
                                         "wheel_emulation");
    setup_num_buttons(ui);
    set_wheel_emulation_button_property_from_preference(mouse_ui);

    set_integer_property_from_preference(mouse_ui,
                                         GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT,
                                         GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT_KEY,
                                         builder,
                                         "middle_button_timeout");
    set_integer_property_from_preference(mouse_ui,
                                         GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT,
                                         GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT_KEY,
                                         builder,
                                         "wheel_emulation_timeout");
    set_integer_property_from_preference(mouse_ui,
                                         GPDS_MOUSE_WHEEL_EMULATION_INERTIA,
                                         GPDS_MOUSE_WHEEL_EMULATION_INERTIA_KEY,
                                         builder,
                                         "wheel_emulation_inertia");

    set_scroll_axes_property_from_preference(mouse_ui,
                                             builder);
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
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

    builder = gpds_ui_get_builder(ui);

    if (!gtk_builder_add_from_file(builder, 
                                   GPDS_MOUSE_UI(ui)->ui_file_path,
                                   error)) {
        return FALSE;
    }

    gpds_ui_set_gconf_string(ui, GPDS_GCONF_DEVICE_TYPE_KEY, "mouse");
    GPDS_MOUSE_UI(ui)->xinput = gpds_mouse_xinput_new(gpds_ui_get_device_name(ui));

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
        return NULL;
    }

    return GTK_WIDGET(widget);
}

static GdkPixbuf *
get_icon_pixbuf (GpdsUI *ui, GError **error)
{
    if (strstr(gpds_ui_get_device_name(ui), "TrackPoint"))
        return gdk_pixbuf_new_from_file(GPDS_ICONDIR "/trackpoint.png", error);
    return gdk_pixbuf_new_from_file(GPDS_ICONDIR "/mouse.png", error);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
