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

#include "gxinput.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
#include <string.h>

typedef struct _GXInputPriv GXInputPriv;
struct _GXInputPriv
{
    gchar *device_name;
    XDeviceInfo *device_info_list;
    XDevice *device;
};

enum
{
    PROP_0,
    PROP_DEVICE_NAME
};

#define G_XINPUT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), G_TYPE_XINPUT, GXInputPriv))

G_DEFINE_TYPE (GXInput, g_xinput, G_TYPE_OBJECT)

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
g_xinput_class_init (GXInputClass *klass)
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

    g_type_class_add_private(gobject_class, sizeof(GXInputPriv));
}

static void
g_xinput_init (GXInput *xinput)
{
    GXInputPriv *priv = G_XINPUT_GET_PRIVATE(xinput);

    priv->device_name = NULL;
    priv->device_info_list = NULL;
    priv->device = NULL;
}

static void
dispose (GObject *object)
{
    GXInputPriv *priv = G_XINPUT_GET_PRIVATE(object);

    g_free(priv->device_name);

    if (priv->device_info_list) {
        XFreeDeviceList(priv->device_info_list);
        priv->device_info_list = NULL;
    }
    
    if (priv->device) {
        XCloseDevice(GDK_DISPLAY(), priv->device);
        priv->device = NULL;
    }

    if (G_OBJECT_CLASS(g_xinput_parent_class)->dispose)
        G_OBJECT_CLASS(g_xinput_parent_class)->dispose(object);
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    GXInputPriv *priv = G_XINPUT_GET_PRIVATE(object);

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
    GXInputPriv *priv = G_XINPUT_GET_PRIVATE(object);

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
g_xinput_error_quark (void)
{
    return g_quark_from_static_string("g-xinput-error-quark");
}

GXInput *
g_xinput_new (const gchar *device_name)
{
    return g_object_new(G_TYPE_XINPUT,
                        "device-name", device_name,
                        NULL);
}

static XDeviceInfo *
get_device_info (const gchar *device_name)
{
    XDeviceInfo *device_infos;
    gint i, n_device_infos;

    device_infos = XListInputDevices(GDK_DISPLAY(), &n_device_infos);

    for (i = 0; i < n_device_infos; i++) {
        if (!strcmp(device_infos[i].name, device_name))
            return &device_infos[i];
    }

    XFreeDeviceList(device_infos);

    return NULL;
}

static XDevice *
open_device (const gchar *device_name, GError **error)
{
    XDeviceInfo *device_info;
    XDevice *device;

    device_info = get_device_info(device_name);
    if (!device_info) {
        g_set_error(error,
                    G_XINPUT_ERROR,
                    G_XINPUT_ERROR_NO_DEVICE,
                    _("No  device found."));
        return NULL;
    }

    device = XOpenDevice(GDK_DISPLAY(), device_info->id);
    if (!device) {
        g_set_error(error,
                    G_XINPUT_ERROR,
                    G_XINPUT_ERROR_NO_DEVICE,
                    _("Could not open %s device."), device_name);
        return NULL;
    }

    return device;
}

static XDevice *
get_device (GXInput *xinput, GError **error)
{
    GXInputPriv *priv = G_XINPUT_GET_PRIVATE(xinput);

    if (priv->device)
        return priv->device;

    priv->device = open_device(priv->device_name, error);
    return priv->device;
}

static gboolean
set_property_va_list (GXInput *xinput,
                      const gchar *property_name,
                      GError **error,
                      gint first_value, va_list var_args)
{
    XDevice *device;
    Atom property_atom;
    gint i, n_values = 1;
    gint *values;
    gint max_value;
    gchar *property_data;
    va_list copy_var_args;
    int format;

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    property_atom = XInternAtom(GDK_DISPLAY(), property_name, False);

    G_VA_COPY(copy_var_args, var_args);
    while (va_arg(var_args, gint))
        n_values++;

    values = g_new(gint, n_values);
    values[0] = first_value;
    max_value = values[0];
    for (i = 1; i < n_values; i++) {
        values[i] = va_arg(copy_var_args, gint);
        max_value = MAX(max_value, values[i]);
    }

    if (max_value <= G_MAXINT8) {
        property_data = (gchar*)g_new(int8_t*, n_values);
        format = 8;
    } else if (max_value <= G_MAXINT16) {
        property_data = (gchar*)g_new(int16_t*, n_values);
        format = 16;
    } else {
        property_data = (gchar*)g_new(int32_t*, n_values);
        format = 32;
    }

    for (i = 0; i < n_values; i++) {
        switch (format) {
        case 8:
            *(((int8_t*)property_data) + i) = values[i];
            break;
        case 16:
            *(((int16_t*)property_data) + i) = values[i];
            break;
        case 32:
        default:
            *(((int32_t*)property_data) + i) = values[i];
            break;
        }
    }

    va_end(copy_var_args);
    g_free(values);

    XChangeDeviceProperty(GDK_DISPLAY(),
                          device, property_atom,
                          XA_INTEGER, format, PropModeReplace,
                          (unsigned char*)property_data, n_values);

    g_free(property_data);

    return TRUE;
}

gboolean
g_xinput_set_property (GXInput *xinput,
                       const gchar *property_name,
                       GError **error,
                       gint first_value, ...)
{
    gboolean success;
    va_list var_args;

    va_start(var_args, first_value);
    success = set_property_va_list(xinput, property_name, error, first_value, var_args);
    va_end(var_args);

    return success;
}

static Atom
get_atom (GXInput *xinput, const gchar *property_name, GError **error)
{
    gint i, n_properties;
    XDevice *device;
    Atom *properties;
    Atom found_atom = -1; 

    device = get_device(xinput, error);
    if (!device)
        return -1;

    properties = XListDeviceProperties(GDK_DISPLAY(), device, &n_properties);
    for (i = 0; i < n_properties; i++) {
        gchar *name;

        name = XGetAtomName(GDK_DISPLAY(), properties[i]);
        if (!strcmp(name, property_name)) {
            found_atom = properties[i];
            break;
        }
    }
    XFree(properties);

    return found_atom;
}

static gboolean
get_int_property (GXInput *xinput,
                  const gchar *property_name,
                  GError **error,
                  gint **values, gulong *n_values)
{
    XDevice *device;
    Atom atom;
    Atom actual_type;
    int actual_format;
    unsigned long bytes_after;
    unsigned char *data, *data_position;
    gulong i;
    gint *int_values;

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    atom = get_atom(xinput, property_name, error);
    if (atom < 0)
        return FALSE;


    if (XGetDeviceProperty(GDK_DISPLAY(), device, atom, 0, 1000, False,
                           XA_INTEGER, &actual_type, &actual_format,
                           n_values, &bytes_after, &data) != Success) {
        return FALSE;
    }

    if (actual_type != XA_INTEGER) {
        XFree(data);
        return FALSE;
    }

    data_position = data;
    int_values = g_new(gint, *n_values);

    for (i = 0; i < *n_values; i++) {
        switch (actual_format) {
        case 8:
            int_values[i] = *((int8_t*)data_position);
            break;
        case 16:
            int_values[i] = *((int16_t*)data_position);
            break;
        case 32:
            int_values[i] = *((int32_t*)data_position);
            break;
        }
        data_position += actual_format / 8;
    }

    *values = int_values;
    XFree(data);

    return TRUE;
}

gboolean
g_xinput_get_property (GXInput *xinput,
                       const gchar *property_name,
                       GError **error,
                       gint **values,
                       gulong *n_values)
{
    XDevice *device;

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    return get_int_property(xinput, property_name, error, values, n_values);
}

gboolean
g_xinput_exist_device (const gchar *device_name)
{
    return get_device_info(device_name) ? TRUE : FALSE;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
