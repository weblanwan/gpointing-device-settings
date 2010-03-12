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

enum {
    DEVICE_NAME_COLUMN,
    ICON_COLUMN,
    N_COLUMNS
};

typedef struct _GpdsMainWindowPriv GpdsMainWindowPriv;
struct _GpdsMainWindowPriv
{
    GList *uis;
};

#define GPDS_MAIN_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_MAIN_WINDOW, GpdsMainWindowPriv))

G_DEFINE_TYPE(GpdsMainWindow, gpds_main_window, GTK_TYPE_DIALOG)

static void     dispose        (GObject          *object);
static void     response       (GtkDialog        *dialog,
                                gint              response);

static void
gpds_main_window_class_init (GpdsMainWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkDialogClass *dialog_class = GTK_DIALOG_CLASS(klass);

    gobject_class->dispose      = dispose;
    dialog_class->response      = response;

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
            priv->uis = g_list_prepend(priv->uis, ui);
            loaded_ui_names = g_list_prepend(loaded_ui_names,
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
        priv->uis = g_list_prepend(priv->uis, ui);
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
    GtkCellRenderer *cell = NULL;
    GtkTreeModel *model;
    GtkNotebook *notebook = GTK_NOTEBOOK(data);
    gint *indices;

    gtk_icon_view_get_cursor(icon_view, &path, &cell);

    if (!path)
        return;
    model = gtk_icon_view_get_model(icon_view);
    indices = gtk_tree_path_get_indices(path);
    gtk_notebook_set_current_page(notebook, indices[0]);

    gtk_tree_path_free(path);
}

static void
gpds_main_window_init (GpdsMainWindow *window)
{
    GpdsMainWindowPriv *priv = GPDS_MAIN_WINDOW_GET_PRIVATE(window);
    GtkWidget *hbox, *vbox;
    GtkWidget *notebook, *icon_view;
    GtkListStore *device_store;

    priv->uis = NULL;
    device_store = gtk_list_store_new(N_COLUMNS,
                                      G_TYPE_STRING,
                                      GDK_TYPE_PIXBUF);

    vbox = gtk_dialog_get_content_area(GTK_DIALOG(window));

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    gtk_widget_show(hbox);

    icon_view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(device_store));
    g_object_unref(device_store);
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(icon_view), DEVICE_NAME_COLUMN);
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(icon_view), ICON_COLUMN);
    gtk_icon_view_set_columns(GTK_ICON_VIEW(icon_view), 1);
    gtk_icon_view_set_item_width(GTK_ICON_VIEW(icon_view), 128);
    gtk_icon_view_set_margin(GTK_ICON_VIEW(icon_view), 0);
    gtk_icon_view_set_column_spacing(GTK_ICON_VIEW(icon_view), 0);
    gtk_icon_view_set_row_spacing(GTK_ICON_VIEW(icon_view), 0);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view), GTK_SELECTION_BROWSE);
    gtk_box_pack_start(GTK_BOX(hbox), icon_view, TRUE, TRUE, 0);
    gtk_widget_show(icon_view);

    notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), notebook, TRUE, TRUE, 0);
    gtk_widget_show(notebook);

    g_signal_connect(icon_view, "selection-changed",
                     G_CALLBACK(cb_selection_changed), notebook);

    append_uis(window, GTK_ICON_VIEW(icon_view), GTK_NOTEBOOK(notebook));

    gtk_dialog_add_buttons(GTK_DIALOG(window),
                           GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                           NULL);

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

    if (G_OBJECT_CLASS(gpds_main_window_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_main_window_parent_class)->dispose(object);
}

static void
response (GtkDialog *dialog, gint response_id)
{
    switch (response_id) {
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
