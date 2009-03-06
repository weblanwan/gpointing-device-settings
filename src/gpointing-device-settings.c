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
#include "gpds-xinput.h"
#include "gpds-module.h"
#include "gpds-ui.h"

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

static void
append_uis (GtkNotebook *notebook)
{
    GList *ui_names, *name;

    ui_names = gpds_uis_get_names();
    
    for (name = ui_names; name; name = g_list_next(name)) {
        GpdsUI *ui;
        GtkWidget *widget = NULL;
        GtkWidget *label = NULL;
        GError *error = NULL;

        ui = gpds_ui_new(name->data, NULL);
        uis = g_list_prepend(uis, ui);

        if (!gpds_ui_is_available(ui, &error)) {
            if (error) {
                g_message("%s", error->message);
                g_clear_error(&error);
            }
            continue;
        }

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
        label = gpds_ui_get_label_widget(ui, &error);
        if (error) {
            g_warning("%s", error->message);
            g_clear_error(&error);
        }

        if (!widget)
            widget = gtk_label_new(error->message);

        gtk_notebook_append_page(notebook,
                                 widget, label);

    }

    g_list_free(ui_names);
}

int
main (int argc, char *argv[])
{
    GtkWidget *dialog, *notebook, *content_area;

    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    gtk_init(&argc, &argv);

    gpds_init();

    dialog = gtk_dialog_new_with_buttons(_("Pointing Device Settings"),
                                         NULL,
                                         0, 
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                         NULL);
    g_signal_connect(dialog, "response",
                     G_CALLBACK(cb_response), NULL);

    notebook = gtk_notebook_new();
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area),
                      notebook);
    append_uis(GTK_NOTEBOOK(notebook));
    gtk_widget_show_all(dialog);
    gtk_main();

    gpds_quit();

    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
