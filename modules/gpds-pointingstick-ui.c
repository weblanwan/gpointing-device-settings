/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2010 Hiroyuki Ikezoe  <poincare@ikezoe.net>
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

#include "gpds-pointingstick-definitions.h"
#include "gpds-pointingstick-xinput.h"

#define GPDS_TYPE_POINTINGSTICK_UI            (gpds_pointingstick_ui_get_type())
#define GPDS_POINTINGSTICK_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_POINTINGSTICK_UI, GpdsMouseUI))
#define GPDS_POINTINGSTICK_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_POINTINGSTICK_UI, GpdsMouseUIClass))
#define G_IS_POINTINGSTICK_UI(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_POINTINGSTICK_UI))
#define G_IS_POINTINGSTICK_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_POINTINGSTICK_UI))
#define GPDS_POINTINGSTICK_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_POINTINGSTICK_UI, GpdsMouseUIClass))

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

GType gpds_pointingstick_ui_get_type (void) G_GNUC_CONST;

static void       dispose            (GObject *object);
static gboolean   is_available       (GpdsUI  *ui, GError **error);
static gboolean   build              (GpdsUI  *ui, GError **error);
static GtkWidget *get_content_widget (GpdsUI  *ui, GError **error);
static GdkPixbuf *get_icon_pixbuf    (GpdsUI  *ui, GError **error);

G_DEFINE_DYNAMIC_TYPE(GpdsMouseUI, gpds_pointingstick_ui, GPDS_TYPE_XINPUT_UI)

static void
gpds_pointingstick_ui_class_init (GpdsMouseUIClass *klass)
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
gpds_pointingstick_ui_class_finalize (GpdsMouseUIClass *klass)
{
}

static void
gpds_pointingstick_ui_init (GpdsMouseUI *ui)
{
    ui->ui_file_path = g_build_filename(gpds_get_ui_file_directory(),
                                        "pointingstick.ui",
                                        NULL);
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_INIT (GTypeModule *type_module)
{
    gpds_pointingstick_ui_register_type(type_module);
}

G_MODULE_EXPORT void
GPDS_MODULE_IMPL_EXIT (void)
{
}

G_MODULE_EXPORT GObject *
GPDS_MODULE_IMPL_INSTANTIATE (const gchar *first_property, va_list var_args)
{
    return g_object_new_valist(GPDS_TYPE_POINTINGSTICK_UI, first_property, var_args);
}

static void
dispose (GObject *object)
{
    GpdsMouseUI *ui = GPDS_POINTINGSTICK_UI(object);

    g_free(ui->ui_file_path);

    if (G_OBJECT_CLASS(gpds_pointingstick_ui_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_pointingstick_ui_parent_class)->dispose(object);
}

static void
show_error (GError *error)
{
    if (!error)
        return;

    g_print("%s\n", error->message);
}

GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(scrolling,
                                             GPDS_POINTINGSTICK_SCROLLING,
                                             "scrolling_box")
GPDS_XINPUT_UI_DEFINE_TOGGLE_BUTTON_CALLBACK(press_to_select,
                                             GPDS_POINTINGSTICK_PRESS_TO_SELECT,
                                             "press_to_select_box")

GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(sensitivity_scale,
                                                   GPDS_POINTINGSTICK_SENSITIVITY)
GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(middle_button_timeout_scale,
                                                   GPDS_POINTINGSTICK_MIDDLE_BUTTON_TIMEOUT)
GPDS_XINPUT_UI_DEFINE_SCALE_VALUE_CHANGED_CALLBACK(press_to_select_threshold_scale,
                                                   GPDS_POINTINGSTICK_PRESS_TO_SELECT_THRESHOLD)

static void
setup_signals (GpdsUI *ui, GtkBuilder *builder)
{
    GObject *object;

#define CONNECT(object_name, signal_name)                               \
    object = gtk_builder_get_object(builder, #object_name);             \
    g_signal_connect(object, #signal_name,                              \
                     G_CALLBACK(cb_ ## object_name ## _ ## signal_name),\
                     ui)

    CONNECT(scrolling, toggled);
    CONNECT(press_to_select, toggled);
    CONNECT(middle_button_timeout_scale, value_changed);
    CONNECT(sensitivity_scale, value_changed);
    CONNECT(press_to_select_threshold_scale, value_changed);

#undef CONNECT
}

static void
setup_current_values (GpdsUI *ui)
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

    SET_BOOLEAN_VALUE(GPDS_POINTINGSTICK_SCROLLING,
                      "scrolling");
    SET_BOOLEAN_VALUE(GPDS_POINTINGSTICK_PRESS_TO_SELECT,
                      "press_to_select");

    SET_INT_VALUE(GPDS_POINTINGSTICK_MIDDLE_BUTTON_TIMEOUT,
                  "middle_button_timeout_scale");
    SET_INT_VALUE(GPDS_POINTINGSTICK_SENSITIVITY,
                  "sensitivity_scale");
    SET_INT_VALUE(GPDS_POINTINGSTICK_PRESS_TO_SELECT_THRESHOLD,
                  "press_to_select_threshold_scale");
}

static gboolean
is_available (GpdsUI *ui, GError **error)
{
    if (GPDS_UI_CLASS(gpds_pointingstick_ui_parent_class)->is_available &&
        !GPDS_UI_CLASS(gpds_pointingstick_ui_parent_class)->is_available(ui, error)) {
        return FALSE;
    }

    if (!g_file_test(GPDS_POINTINGSTICK_UI(ui)->ui_file_path,
                     G_FILE_TEST_EXISTS)) {
        g_set_error(error,
                    GPDS_UI_ERROR,
                    GPDS_UI_ERROR_NO_UI_FILE,
                    _("%s did not find."),
                    GPDS_POINTINGSTICK_UI(ui)->ui_file_path);
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
                                   GPDS_POINTINGSTICK_UI(ui)->ui_file_path,
                                   error)) {
        return FALSE;
    }

    xinput = gpds_pointingstick_xinput_new(gpds_ui_get_device_name(ui));
    if (!xinput) {
        return FALSE;
    }
    gpds_xinput_ui_set_xinput(GPDS_XINPUT_UI(ui), xinput);
    g_object_unref(xinput);

    gpds_ui_set_gconf_string(ui, GPDS_GCONF_DEVICE_TYPE_KEY, "pointingstick");
    setup_current_values(ui);
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
    gchar *path;
    GdkPixbuf *pixbuf;

    path = g_build_filename(gpds_get_icon_file_directory(),
                            "trackpoint.png", NULL);
    pixbuf = gdk_pixbuf_new_from_file(path, error);
    g_free(path);

    return pixbuf;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
