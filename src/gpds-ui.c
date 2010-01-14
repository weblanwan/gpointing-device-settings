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
#include <gconf/gconf-client.h>
#include "gpds-module.h"
#include "gpds-gconf.h"

static GList *uis = NULL;

gboolean
gpds_uis_load (void)
{
    uis = gpds_module_load_modules();
    return TRUE;
}

gboolean
gpds_uis_unload (void)
{
    g_list_foreach(uis, (GFunc)gpds_module_unload, NULL);
    g_list_free(uis);

    return TRUE;
}

GList *
gpds_uis_get_names (void)
{
    return gpds_module_collect_names(uis);
}

typedef struct _GpdsUIPriv GpdsUIPriv;
struct _GpdsUIPriv
{
    GtkBuilder *builder;
    gchar *device_name;
    GConfClient *gconf;
};

enum
{
    PROP_0,
    PROP_DEVICE_NAME
};

#define GPDS_UI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_UI, GpdsUIPriv))

G_DEFINE_ABSTRACT_TYPE(GpdsUI, gpds_ui, G_TYPE_OBJECT)

static void dispose      (GObject      *object);
static void set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec);
static void get_property (GObject      *object,
                          guint         prop_id,
                          GValue       *value,
                          GParamSpec   *pspec);

static void
gpds_ui_class_init (GpdsUIClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = dispose;
    gobject_class->set_property = set_property;
    gobject_class->get_property = get_property;

    g_object_class_install_property
        (gobject_class,
         PROP_DEVICE_NAME,
         g_param_spec_string("device-name",
             "Device Name",
             "The device name",
             NULL,
             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_type_class_add_private(gobject_class, sizeof(GpdsUIPriv));
}

static void
gpds_ui_init (GpdsUI *ui)
{
    GpdsUIPriv *priv = GPDS_UI_GET_PRIVATE(ui);

    priv->device_name = NULL;
    priv->builder = gtk_builder_new();
    priv->gconf = gconf_client_get_default();
}

static void
dispose (GObject *object)
{
    GpdsUIPriv *priv = GPDS_UI_GET_PRIVATE(object);

    g_free(priv->device_name);

    if (priv->builder) {
        g_object_unref(priv->builder);
        priv->builder = NULL;
    }

    if (priv->gconf) {
        g_object_unref(priv->gconf);
        priv->gconf = NULL;
    }

    if (G_OBJECT_CLASS(gpds_ui_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_ui_parent_class)->dispose(object);
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    GpdsUIPriv *priv = GPDS_UI_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_DEVICE_NAME:
        g_free(priv->device_name);
        priv->device_name = g_value_dup_string(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
    GpdsUIPriv *priv = GPDS_UI_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_DEVICE_NAME:
        g_value_set_string(value, priv->device_name);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

GQuark
gpds_ui_error_quark (void)
{
    return g_quark_from_static_string("gpds-ui-error-quark");
}

GpdsUI *
gpds_ui_new (const gchar *name, const gchar *first_property, ...)
{
    GpdsModule *module;
    GObject *ui;
    va_list var_args;

    module = gpds_module_find(uis, name);
    if (!module)
    {
        module = gpds_module_load_module(gpds_module_directory(), name);
        g_return_val_if_fail(module != NULL, NULL);

        uis = g_list_prepend(uis, module);
    }
    va_start(var_args, first_property);
    ui = gpds_module_instantiate(module, first_property, var_args);
    va_end(var_args);

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

GdkPixbuf *
gpds_ui_get_icon_pixbuf (GpdsUI *ui, GError **error)
{
    GpdsUIClass *klass;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    klass = GPDS_UI_GET_CLASS(ui);
    return (klass->get_icon_pixbuf) ? klass->get_icon_pixbuf(ui, error) : FALSE;
}

GtkBuilder *
gpds_ui_get_builder (GpdsUI *ui)
{
    g_return_val_if_fail(GPDS_IS_UI(ui), NULL);

    return GPDS_UI_GET_PRIVATE(ui)->builder;
}

GObject *
gpds_ui_get_ui_object_by_name (GpdsUI *ui, const gchar *name)
{
    GpdsUIPriv *priv;

    g_return_val_if_fail(GPDS_IS_UI(ui), NULL);

    priv = GPDS_UI_GET_PRIVATE(ui);
    g_return_val_if_fail(priv->builder, NULL);

    return gtk_builder_get_object(priv->builder, name);
}

const gchar *
gpds_ui_get_device_name (GpdsUI *ui)
{
    g_return_val_if_fail(GPDS_IS_UI(ui), NULL);

    return GPDS_UI_GET_PRIVATE(ui)->device_name;
}

static gchar *
build_gconf_key (GpdsUI *ui, const gchar *key)
{
    gchar *gconf_key;
    gchar *device_name;
    GpdsUIPriv *priv = GPDS_UI_GET_PRIVATE(ui);

    device_name = gconf_escape_key(priv->device_name, -1);
    gconf_key = g_strdup_printf("%s/%s/%s",
                                GPDS_GCONF_DIR,
                                device_name,
                                key);
    g_free(device_name);

    return gconf_key;
}

void
gpds_ui_set_gconf_bool (GpdsUI *ui, const gchar *key, gboolean value)
{
    gchar *gconf_key;
    GpdsUIPriv *priv;

    g_return_if_fail(GPDS_IS_UI(ui));

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    gconf_client_set_bool(priv->gconf, gconf_key, value, NULL);
    g_free(gconf_key);
}

gboolean
gpds_ui_get_gconf_bool (GpdsUI *ui, const gchar *key, gboolean *value)
{
    gchar *gconf_key;
    gboolean exist_value = FALSE;
    GpdsUIPriv *priv;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    exist_value =gpds_gconf_get_boolean(priv->gconf, gconf_key, value);
    g_free(gconf_key);

    return exist_value;
}

void
gpds_ui_set_gconf_int (GpdsUI *ui, const gchar *key, gint value)
{
    gchar *gconf_key;
    GpdsUIPriv *priv;

    g_return_if_fail(GPDS_IS_UI(ui));

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    gconf_client_set_int(priv->gconf, gconf_key, value, NULL);
    g_free(gconf_key);
}

gboolean
gpds_ui_get_gconf_int (GpdsUI *ui, const gchar *key, gint *value)
{
    gchar *gconf_key;
    gboolean exist_value = FALSE;
    GpdsUIPriv *priv;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    exist_value =gpds_gconf_get_int(priv->gconf, gconf_key, value);
    g_free(gconf_key);

    return exist_value;
}

void
gpds_ui_set_gconf_float (GpdsUI *ui, const gchar *key, gdouble value)
{
    gchar *gconf_key;
    GpdsUIPriv *priv;

    g_return_if_fail(GPDS_IS_UI(ui));

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    gconf_client_set_float(priv->gconf, gconf_key, value, NULL);
    g_free(gconf_key);
}

gboolean
gpds_ui_get_gconf_float (GpdsUI *ui, const gchar *key, gdouble *value)
{
    gchar *gconf_key;
    gboolean exist_value = FALSE;
    GpdsUIPriv *priv;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    exist_value =gpds_gconf_get_float(priv->gconf, gconf_key, value);
    g_free(gconf_key);

    return exist_value;
}

void
gpds_ui_set_gconf_string (GpdsUI *ui, const gchar *key, const gchar *value)
{
    gchar *gconf_key;
    GpdsUIPriv *priv;

    g_return_if_fail(GPDS_IS_UI(ui));

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    gconf_client_set_string(priv->gconf, gconf_key, value, NULL);
    g_free(gconf_key);
}

gboolean
gpds_ui_get_gconf_string (GpdsUI *ui, const gchar *key, gchar **value)
{
    gchar *gconf_key;
    gboolean exist_value = FALSE;
    GpdsUIPriv *priv;

    g_return_val_if_fail(GPDS_IS_UI(ui), FALSE);

    priv = GPDS_UI_GET_PRIVATE(ui);
    gconf_key = build_gconf_key(ui, key);
    exist_value =gpds_gconf_get_string(priv->gconf, gconf_key, value);
    g_free(gconf_key);

    return exist_value;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
