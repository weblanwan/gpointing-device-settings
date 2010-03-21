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
#include "gpds-main-window.h"
#include "gpds-utils.h"
#include "gpds-ui.h"
#include "gpds-xinput-pointer-info.h"
#include "gpds-ui.h"
#include "gpds-utils.h"
#include "gpds-grayed-desktop.h"
#include "gpds-event-feedback.h"

enum {
    DEVICE_NAME_COLUMN,
    ICON_COLUMN,
    ORIGINAL_ICON_COLUMN,
    N_COLUMNS
};

typedef struct _GpdsMainWindowPriv GpdsMainWindowPriv;
struct _GpdsMainWindowPriv
{
    GList *uis;
    GtkWidget *background;
    GtkWidget *dry_run_button;
    GtkWidget *feedback;
    GtkIconView *icon_view;
    GdkColor *original_text_color;
    GdkColor *original_base_color;
    guint timeout_id;
    gboolean heartbeat;
    gboolean dry_run;
};

#define GPDS_MAIN_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_MAIN_WINDOW, GpdsMainWindowPriv))

G_DEFINE_TYPE(GpdsMainWindow, gpds_main_window, GTK_TYPE_DIALOG)

static void     dispose        (GObject          *object);
static void     response       (GtkDialog        *dialog,
                                gint              response);
static gboolean button_press   (GtkWidget        *widget,
                                GdkEventButton   *event);
static gboolean button_release (GtkWidget        *widget,
                                GdkEventButton   *event);
static gboolean motion_notify  (GtkWidget        *widget,
                                GdkEventMotion   *event);
static gboolean scroll         (GtkWidget        *widget,
                                GdkEventScroll   *event);

static void
gpds_main_window_class_init (GpdsMainWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkDialogClass *dialog_class = GTK_DIALOG_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gobject_class->dispose      = dispose;
    dialog_class->response      = response;

    widget_class->motion_notify_event  = motion_notify;
    widget_class->button_press_event   = button_press;
    widget_class->button_release_event = button_release;
    widget_class->scroll_event         = scroll;

    g_type_class_add_private(gobject_class, sizeof(GpdsMainWindowPriv));
}

static void
append_ui (GtkIconView *icon_view, GtkNotebook *notebook,
           GpdsUI *ui)
{
    GtkWidget *widget = NULL;
    GError *error = NULL;
    GtkTreeIter iter;
    GdkPixbuf *pixbuf;
    GtkListStore *list_store;
    const gchar *device_name;

    gpds_ui_build(ui, &error);
    if (error) {
        g_warning("%s", error->message);
        g_clear_error(&error);
    }
    widget = gpds_ui_get_content_widget(ui, &error);
    if (error) {
        g_warning("%s", error->message);
        g_clear_error(&error);
    }

    if (!widget)
        widget = gtk_label_new(error->message);

    list_store = GTK_LIST_STORE(gtk_icon_view_get_model(icon_view));
    gtk_list_store_append(list_store, &iter);
    pixbuf = gpds_ui_get_icon_pixbuf(ui, &error);
    if (error) {
        g_warning("%s", error->message);
        g_clear_error(&error);
    }

    device_name = gpds_ui_get_device_name(ui);
    gtk_list_store_set(list_store, &iter,
                       DEVICE_NAME_COLUMN, device_name,
                       ICON_COLUMN, pixbuf,
                       ORIGINAL_ICON_COLUMN, pixbuf,
                       -1);
    gtk_notebook_append_page(notebook, widget, NULL);
    gtk_widget_show_all(widget);
    if (pixbuf)
        g_object_unref(pixbuf);
}

static void
select_first_device (GtkIconView *icon_view)
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;

    model = gtk_icon_view_get_model(icon_view);
    gtk_tree_model_get_iter_first(model, &iter);
    path = gtk_tree_model_get_path(model, &iter);
    gtk_icon_view_select_path(icon_view, path);
    gtk_tree_path_free(path);
}

static GpdsUI *
create_ui_from_pointer_info (GpdsXInputPointerInfo *info)
{
    GpdsUI *ui;
    gchar *type_name;
    const gchar *device_name;
    GError *error = NULL;

    device_name = gpds_xinput_pointer_info_get_name(info);
    if (!strcmp(device_name, "Macintosh mouse button emulation"))
        return NULL;

    type_name = g_ascii_strdown(gpds_xinput_pointer_info_get_type_name(info), -1);
    ui = gpds_ui_new(type_name,
                     "device-name", device_name,
                     NULL);
    g_free(type_name);
    if (!ui)
        return NULL;

    if (!gpds_ui_is_available(ui, &error)) {
        if (error) {
            g_message("%s", error->message);
            g_clear_error(&error);
        }
        g_object_unref(ui);
        return NULL;
    }

    return ui;
}

static void
append_uis (GpdsMainWindow *window, GtkIconView *icon_view, GtkNotebook *notebook)
{
    GpdsUI *ui;
    GList *node, *pointer_infos;;
    GList *ui_names, *loaded_ui_names = NULL;
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);

    pointer_infos = gpds_xinput_utils_collect_pointer_infos();
    for (node = pointer_infos; node; node = g_list_next(node)) {
        GpdsXInputPointerInfo *info = node->data;

        ui = create_ui_from_pointer_info(info);
        if (ui) {
            priv->uis = g_list_append(priv->uis, ui);
            loaded_ui_names = g_list_append(loaded_ui_names,
                                            g_strdup(gpds_xinput_pointer_info_get_type_name(info)));
            append_ui(icon_view, notebook, ui);
        }
    }

    g_list_foreach(pointer_infos, (GFunc)gpds_xinput_pointer_info_free, NULL);
    g_list_free(pointer_infos);

    ui_names = gpds_uis_get_names();
    for (node = ui_names; node; node = g_list_next(node)) {
        const gchar *ui_name = node->data;

        if (g_list_find_custom(loaded_ui_names, ui_name,
                               (GCompareFunc)g_str_equal)) {
            continue;
        }

        ui = gpds_ui_new(ui_name, NULL);
        if (!gpds_ui_is_available(ui, NULL)) {
            g_object_unref(ui);
            continue;
        }
        priv->uis = g_list_append(priv->uis, ui);
        append_ui(icon_view, notebook, ui);
    }
    g_list_free(ui_names);
    g_list_foreach(loaded_ui_names, (GFunc)g_free, NULL);
    g_list_free(loaded_ui_names);

    select_first_device(icon_view);
}

static void
cb_selection_changed (GtkIconView *icon_view, gpointer data)
{
    GtkTreePath *path = NULL;
    GtkNotebook *notebook = GTK_NOTEBOOK(data);
    gint *indices;

    gtk_icon_view_get_cursor(icon_view, &path, NULL);

    if (!path)
        return;
    indices = gtk_tree_path_get_indices(path);
    gtk_notebook_set_current_page(notebook, indices[0]);

    gtk_tree_path_free(path);
}

static gboolean
cb_button_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    gtk_widget_event(GTK_WIDGET(user_data), (GdkEvent*)event);
    return FALSE;
}

static gboolean
cb_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    gtk_widget_event(GTK_WIDGET(user_data), (GdkEvent*)event);
    return FALSE;
}

static gboolean
cb_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
    gtk_widget_event(GTK_WIDGET(user_data), (GdkEvent*)event);
    return FALSE;
}

static void
gpds_main_window_init (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);
    GtkWidget *hbox, *vbox;
    GtkWidget *notebook;
    GtkListStore *device_store;

    priv->uis = NULL;
    priv->timeout_id = 0;
    priv->heartbeat = FALSE;
    priv->dry_run = FALSE;
    priv->background = NULL;
    priv->original_text_color = NULL;
    priv->original_base_color = NULL;
    priv->feedback = gpds_event_feedback_new(GTK_WINDOW(window));

    device_store = gtk_list_store_new(N_COLUMNS,
                                      G_TYPE_STRING,
                                      GDK_TYPE_PIXBUF,
                                      GDK_TYPE_PIXBUF);

    vbox = gtk_dialog_get_content_area(GTK_DIALOG(window));

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    gtk_widget_show(hbox);

    priv->icon_view = GTK_ICON_VIEW(gtk_icon_view_new_with_model(GTK_TREE_MODEL(device_store)));
    g_object_unref(device_store);
    gtk_icon_view_set_text_column(priv->icon_view, DEVICE_NAME_COLUMN);
    gtk_icon_view_set_pixbuf_column(priv->icon_view, ICON_COLUMN);
    gtk_icon_view_set_columns(priv->icon_view, 1);
    gtk_icon_view_set_item_width(priv->icon_view, 128);
    gtk_icon_view_set_margin(priv->icon_view, 0);
    gtk_icon_view_set_column_spacing(priv->icon_view, 0);
    gtk_icon_view_set_row_spacing(priv->icon_view, 0);
    gtk_icon_view_set_selection_mode(priv->icon_view, GTK_SELECTION_BROWSE);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(priv->icon_view), TRUE, TRUE, 0);
    gtk_widget_show(GTK_WIDGET(priv->icon_view));

    notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), notebook, TRUE, TRUE, 0);
    gtk_widget_show(notebook);

    g_signal_connect(priv->icon_view, "selection-changed",
                     G_CALLBACK(cb_selection_changed), notebook);

    append_uis(window, priv->icon_view, GTK_NOTEBOOK(notebook));

    gtk_dialog_add_button(GTK_DIALOG(window),
                          GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
    priv->dry_run_button = gtk_dialog_add_button(GTK_DIALOG(window),
                                                 _("Dry _run"),   1);
    gtk_dialog_add_button(GTK_DIALOG(window),
                           GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);

    gtk_window_set_default_icon_name("preferences-desktop-peripherals");
}

static void
dispose (GObject *object)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(object);

    if (priv->uis) {
        g_list_foreach(priv->uis, (GFunc)g_object_unref, NULL);
        g_list_free(priv->uis);
        priv->uis = NULL;
    }

    if (priv->original_text_color) {
        gdk_color_free(priv->original_text_color);
        priv->original_text_color = NULL;
    }

    if (priv->original_base_color) {
        gdk_color_free(priv->original_base_color);
        priv->original_base_color = NULL;
    }

    if (G_OBJECT_CLASS(gpds_main_window_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_main_window_parent_class)->dispose(object);
}

static void
heartbeat (GtkWidget *widget)
{
    GPDS_MAIN_WINDOW_GET_PRIVATE(widget)->heartbeat = TRUE;
}

static gboolean
restore_original_pixbuf (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(data);
    GdkPixbuf *pixbuf = NULL;

    if (gtk_icon_view_path_is_selected(priv->icon_view, path))
        return FALSE;

    gtk_tree_model_get(model, iter,
                       ORIGINAL_ICON_COLUMN, &pixbuf,
                       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       ICON_COLUMN, pixbuf,
                       -1);
    g_object_unref(pixbuf);

    return FALSE;
}

static GpdsUI *
get_current_ui (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);
    GtkTreePath *path = NULL;
    GList *selected;
    gint *indices, index;

    selected = gtk_icon_view_get_selected_items(priv->icon_view);

    path = (GtkTreePath*)g_list_first(selected)->data;
    indices = gtk_tree_path_get_indices(path);
    index = indices[0];
    g_list_foreach(selected, (GFunc)gtk_tree_path_free, NULL);
    g_list_free(selected);

    return GPDS_UI(g_list_nth_data(priv->uis, index));
}

static gboolean
button_press (GtkWidget *widget, GdkEventButton *event)
{
    heartbeat(widget);

    return FALSE;
}

static gboolean
button_release (GtkWidget *widget, GdkEventButton *event)
{
    heartbeat(widget);

    return FALSE;
}

static gboolean
motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
    heartbeat(widget);

    return FALSE;
}

static gboolean
scroll (GtkWidget *widget, GdkEventScroll *event)
{
    heartbeat(widget);

    return FALSE;
}

static void
disconnect_icon_view_signals (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);

    g_signal_handlers_disconnect_by_func(priv->icon_view,
                                         G_CALLBACK(cb_button_event), window);
    g_signal_handlers_disconnect_by_func(priv->icon_view,
                                         G_CALLBACK(cb_motion_notify), window);
    g_signal_handlers_disconnect_by_func(priv->icon_view,
                                         G_CALLBACK(cb_scroll), window);
}

static void
finish_dry_run (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);
    GtkTreeModel *model;
    GtkStyle *style;
    GpdsUI *ui;

    ui = get_current_ui(window);
    if (!ui)
        return;

    priv->dry_run = FALSE;
    gtk_button_set_label(GTK_BUTTON(priv->dry_run_button),
                         _("Dry _run"));

    gpds_ui_finish_dry_run(ui, NULL);

    disconnect_icon_view_signals(window);
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
    if (priv->background) {
        gtk_widget_destroy(priv->background);
        priv->background = NULL;
    }
    gtk_icon_view_set_selection_mode(priv->icon_view, GTK_SELECTION_BROWSE);
    model = gtk_icon_view_get_model(priv->icon_view);
    gtk_tree_model_foreach(model, restore_original_pixbuf, window);

    style = gtk_widget_get_style(GTK_WIDGET(priv->icon_view));
    gtk_widget_modify_text(GTK_WIDGET(priv->icon_view),
                           GTK_STATE_NORMAL, priv->original_text_color);
    gtk_widget_modify_base(GTK_WIDGET(priv->icon_view),
                           GTK_STATE_NORMAL, priv->original_base_color);
    gtk_widget_queue_draw(GTK_WIDGET(priv->icon_view));

}

static gboolean
cb_timeout (gpointer data)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(data);

    if (!priv->heartbeat) {
        finish_dry_run(GPDS_MAIN_WINDOW(data));
        return FALSE;
    }

    priv->heartbeat = FALSE;

    return TRUE;
}

static gboolean
grab_pointer (GpdsMainWindow *window)
{
    GdkGrabStatus status;
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);

    status = gdk_pointer_grab(gtk_widget_get_window(GTK_WIDGET(window)),
                              TRUE,
                              GDK_POINTER_MOTION_MASK |
                              GDK_BUTTON_PRESS_MASK |
                              GDK_BUTTON_RELEASE_MASK |
                              GDK_SCROLL_MASK,
                              NULL,
                              NULL,
                              GDK_CURRENT_TIME);
    if (status != GDK_GRAB_SUCCESS)
        return FALSE;

    priv->timeout_id = g_timeout_add_seconds(3, cb_timeout, window);

    return TRUE;
}

static gboolean
set_grayscaled_pixbuf (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(data);
    GdkPixbuf *pixbuf = NULL;
    GdkPixbuf *gray;

    if (gtk_icon_view_path_is_selected(priv->icon_view, path))
        return FALSE;

    gtk_tree_model_get(model, iter,
                       ORIGINAL_ICON_COLUMN, &pixbuf,
                       -1);
    gray = gpds_convert_to_grayscaled_pixbuf(pixbuf);

    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       ICON_COLUMN, gray,
                       -1);
    g_object_unref(pixbuf);

    return FALSE;
}

static void
connect_icon_view_signals (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);

    g_signal_connect(priv->icon_view, "button-press-event",
                     G_CALLBACK(cb_button_event), window);
    g_signal_connect(priv->icon_view, "button-release-event",
                     G_CALLBACK(cb_button_event), window);
    g_signal_connect(priv->icon_view, "motion-notify-event",
                     G_CALLBACK(cb_motion_notify), window);
    g_signal_connect(priv->icon_view, "scroll-event",
                     G_CALLBACK(cb_scroll), window);
}

static void
start_dry_run (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);
    GtkTreeModel *model;
    GtkStyle *style;
    GpdsUI *ui;

    ui = get_current_ui(window);
    if (!ui)
        return;
    priv->dry_run = TRUE;
    gpds_ui_dry_run(ui, NULL);

    gtk_button_set_label(GTK_BUTTON(priv->dry_run_button),
                         _("_Finish dry run"));

    priv->background = gpds_grayed_desktop_new(GTK_WINDOW(window));
    gtk_widget_show(priv->background);

    model = gtk_icon_view_get_model(priv->icon_view);
    gtk_tree_model_foreach(model, set_grayscaled_pixbuf, window);
    gtk_icon_view_set_selection_mode(priv->icon_view, GTK_SELECTION_NONE);

    style = gtk_widget_get_style(GTK_WIDGET(priv->icon_view));

    if (priv->original_text_color)
        gdk_color_free(priv->original_text_color);
    priv->original_text_color = gdk_color_copy(&style->text[GTK_STATE_NORMAL]);
    if (priv->original_base_color)
        gdk_color_free(priv->original_base_color);
    priv->original_base_color = gdk_color_copy(&style->base[GTK_STATE_NORMAL]);

    gtk_widget_modify_text(GTK_WIDGET(priv->icon_view),
                           GTK_STATE_NORMAL, &style->text[GTK_STATE_INSENSITIVE]);
    gtk_widget_modify_base(GTK_WIDGET(priv->icon_view),
                           GTK_STATE_NORMAL, &style->base[GTK_STATE_INSENSITIVE]);
    gtk_widget_queue_draw(GTK_WIDGET(priv->icon_view));
    connect_icon_view_signals(window);

    grab_pointer(window);
}

static void
apply (GpdsUI *ui, gpointer user_data)
{
    gpds_ui_apply(ui, NULL);
}

static void
apply_all (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);

    g_list_foreach(priv->uis, (GFunc)apply, NULL);
}

static void
response (GtkDialog *dialog, gint response_id)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(dialog);
    switch (response_id) {
    case 1:
        if (!priv->dry_run)
            start_dry_run(GPDS_MAIN_WINDOW(dialog));
        else
            finish_dry_run(GPDS_MAIN_WINDOW(dialog));
        break;
    case GTK_RESPONSE_APPLY:
        apply_all(GPDS_MAIN_WINDOW(dialog));
        break;
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CLOSE:
    default:
        gtk_main_quit();
        break;
    }
}

GtkWidget *
gpds_main_window_new (void)
{
    return GTK_WIDGET(g_object_new(GPDS_TYPE_MAIN_WINDOW, NULL));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
