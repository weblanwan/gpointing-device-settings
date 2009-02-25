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

#include "gpds-ui.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gpds-module.h"
#include "gpds-xinput.h"

typedef struct _GpdsUIPriv GpdsUIPriv;
struct _GpdsUIPriv
{
    GtkBuilder *builder;
    GpdsXInput *xinput;
};


#define GPDS_UI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_UI, GpdsUIPriv))

G_DEFINE_ABSTRACT_TYPE(GpdsUI, gpds_ui, G_TYPE_OBJECT)

static void dispose      (GObject      *object);

static void
gpds_ui_class_init (GpdsUIClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = dispose;

    g_type_class_add_private(gobject_class, sizeof(GpdsUIPriv));
}

static void
gpds_ui_init (GpdsUI *ui)
{
    GpdsUIPriv *priv = GPDS_UI_GET_PRIVATE(ui);

    priv->xinput = NULL;
    priv->builder = gtk_builder_new();
}

static void
dispose (GObject *object)
{
    GpdsUIPriv *priv = GPDS_UI_GET_PRIVATE(object);

    if (priv->builder) {
        g_object_unref(priv->builder);
        priv->builder = NULL;
    }

    if (priv->xinput) {
        g_object_unref(priv->xinput);
        priv->xinput = NULL;
    }

    if (G_OBJECT_CLASS(gpds_ui_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_ui_parent_class)->dispose(object);
}

GQuark
gpds_ui_error_quark (void)
{
    return g_quark_from_static_string("gpds-ui-error-quark");
}

GpdsUI *
gpds_ui_new (const gchar *name)
{
    GpdsModule *module;
    GObject *ui;

    module = gpds_module_load_module(gpds_module_directory(), name);
    g_return_val_if_fail(module != NULL, NULL);

    ui = gpds_module_instantiate(module);

    return GPDS_UI(ui);
}

gboolean
gpds_ui_is_available (GpdsUI *ui, GError **error)
{
    GpdsUIClass *klass;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    klass = GPDS_UI_GET_CLASS(ui);
    return (klass->is_available) ? klass->is_available(ui, error) : FALSE;
}

gboolean
gpds_ui_build (GpdsUI *ui, GError **error)
{
    GpdsUIClass *klass;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    klass = GPDS_UI_GET_CLASS(ui);
    return (klass->build) ? klass->build(ui, error) : FALSE;
}

GtkWidget *
gpds_ui_get_content_widget (GpdsUI *ui, GError **error)
{
    GpdsUIClass *klass;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    klass = GPDS_UI_GET_CLASS(ui);
    return (klass->get_content_widget) ? klass->get_content_widget(ui, error) : FALSE;
}

GtkWidget *
gpds_ui_get_label_widget (GpdsUI *ui, GError **error)
{
    GpdsUIClass *klass;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    klass = GPDS_UI_GET_CLASS(ui);
    return (klass->get_label_widget) ? klass->get_label_widget(ui, error) : FALSE;
}


GtkBuilder *
gpds_ui_get_builder (GpdsUI *ui)
{
    return GPDS_UI_GET_PRIVATE(ui)->builder;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
