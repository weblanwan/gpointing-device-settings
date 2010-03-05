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
#include <stdlib.h>
#include "gpds-xinput-pointer-info.h"
#include "gpds-module.h"
#include "gpds-ui.h"
#include "gpds-utils.h"

enum {
    DEVICE_NAME_COLUMN,
    ICON_COLUMN,
    N_COLUMNS
};

static GList *uis = NULL;

static gboolean
gpds_init (void)
{
    gpds_uis_load();
    return TRUE;
}

static gboolean
gpds_quit (void)
{
    g_list_foreach(uis, (GFunc)g_object_unref, NULL);
    g_list_free(uis);
    gpds_uis_unload();

    return TRUE;
}

static void
cb_response (GtkDialog *dialog, gint response_id, gpointer user_data)
{
    gtk_main_quit();
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

static void
append_uis (GtkIconView *icon_view, GtkNotebook *notebook)
{
    GpdsUI *ui;
    GList *node, *pointer_infos;;
    GList *ui_names, *loaded_ui_names = NULL;

    pointer_infos = gpds_xinput_utils_collect_pointer_infos();
    for (node = pointer_infos; node; node = g_list_next(node)) {
        GpdsXInputPointerInfo *info = node->data;

        ui = create_ui_from_pointer_info(info);
        if (ui) {
            uis = g_list_prepend(uis, ui);
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
        uis = g_list_prepend(uis, ui);
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

int
main (int argc, char *argv[])
{
    GtkWidget *dialog;
    GObject *notebook, *icon_view;
    GtkBuilder *builder;
    GError *error = NULL;
    gchar *ui_file_path;

    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    ui_file_path = g_build_filename(gpds_get_ui_file_directory(),
                                    "main-window.ui",
                                     NULL);
    gtk_builder_add_from_file(builder, ui_file_path, &error);
    g_free(ui_file_path);
    if (error) {
        g_print("%s\n", error->message);
        exit(EXIT_FAILURE);
    }

    gpds_init();

    dialog = GTK_WIDGET(gtk_builder_get_object(builder, "main-window"));
    gtk_window_set_default_icon_name("preferences-desktop-peripherals");
    g_signal_connect(dialog, "response",
                     G_CALLBACK(cb_response), NULL);

    notebook = gtk_builder_get_object(builder, "device-notebook");
    icon_view = gtk_builder_get_object(builder, "device-icon-view");
    g_signal_connect(icon_view, "selection-changed",
                     G_CALLBACK(cb_selection_changed), notebook);

    append_uis(GTK_ICON_VIEW(icon_view), GTK_NOTEBOOK(notebook));
    g_object_unref(builder);

    gtk_widget_show_all(dialog);
    gtk_main();

    gpds_quit();

    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
