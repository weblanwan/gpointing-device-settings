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

#include "gpointing-device-settings.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gtrackpoint-ui.h"
#include "gxinput.h"

static void
cb_response (GtkDialog *dialog, gint response_id, gpointer user_data)
{
    gtk_main_quit();
}

int
main (int argc, char *argv[])
{
    GTrackPointUI *ui;
    GtkWidget *dialog, *notebook, *content_area;
    GtkWidget *widget;

    gtk_init(&argc, &argv);

    dialog = gtk_dialog_new_with_buttons(_("TrackPoint Settings"),
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

    ui = g_track_point_ui_new();
    widget = g_track_point_ui_get_widget(ui);
    if (!widget)
        widget = gtk_label_new(_("No device availlable"));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             widget, NULL);

    gtk_widget_show_all(dialog);
    gtk_main();

    g_object_unref(ui);

    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
