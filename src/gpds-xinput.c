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

#include "gpds-xinput.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
#include <string.h>
#include "gpds-xinput-utils.h"

typedef struct _GpdsXInputPriv GpdsXInputPriv;
struct _GpdsXInputPriv
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

#define GPDS_XINPUT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_XINPUT, GpdsXInputPriv))

G_DEFINE_TYPE (GpdsXInput, gpds_xinput, G_TYPE_OBJECT)

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
gpds_xinput_class_init (GpdsXInputClass *klass)
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

    g_type_class_add_private(gobject_class, sizeof(GpdsXInputPriv));
}

static void
gpds_xinput_init (GpdsXInput *xinput)
{
    GpdsXInputPriv *priv = GPDS_XINPUT_GET_PRIVATE(xinput);

    priv->device_name = NULL;
    priv->device_info_list = NULL;
    priv->device = NULL;
}

static void
dispose (GObject *object)
{
    GpdsXInputPriv *priv = GPDS_XINPUT_GET_PRIVATE(object);

    g_free(priv->device_name);

    if (priv->device_info_list) {
        XFreeDeviceList(priv->device_info_list);
        priv->device_info_list = NULL;
    }
    
    if (priv->device) {
        XCloseDevice(GDK_DISPLAY(), priv->device);
        priv->device = NULL;
    }

    if (G_OBJECT_CLASS(gpds_xinput_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_xinput_parent_class)->dispose(object);
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    GpdsXInputPriv *priv = GPDS_XINPUT_GET_PRIVATE(object);

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
    GpdsXInputPriv *priv = GPDS_XINPUT_GET_PRIVATE(object);

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
gpds_xinput_error_quark (void)
{
    return g_quark_from_static_string("gpds-xinput-error-quark");
}

GpdsXInput *
gpds_xinput_new (const gchar *device_name)
{
    g_return_val_if_fail(device_name, NULL);

    return g_object_new(GPDS_TYPE_XINPUT,
                        "device-name", device_name,
                        NULL);
}

const gchar *
gpds_xinput_get_device_name (GpdsXInput *xinput)
{
    g_return_val_if_fail(GPDS_IS_XINPUT(xinput), NULL);

    return GPDS_XINPUT_GET_PRIVATE(xinput)->device_name;
}

static XDevice *
get_device (GpdsXInput *xinput, GError **error)
{
    GpdsXInputPriv *priv = GPDS_XINPUT_GET_PRIVATE(xinput);

    if (priv->device)
        return priv->device;

    priv->device = gpds_xinput_utils_open_device(priv->device_name, error);
    return priv->device;
}

gboolean
gpds_xinput_set_int_properties (GpdsXInput *xinput,
                                const gchar *property_name,
                                gint format_type,
                                GError **error,
                                gint *properties,
                                guint n_properties)
{
    XDevice *device;
    Atom property_atom;
    gint i;
    gchar *property_data;

    g_return_val_if_fail(GPDS_IS_XINPUT(xinput), FALSE);

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    property_atom = XInternAtom(GDK_DISPLAY(), property_name, False);

    switch (format_type) {
    case 8:
        property_data = (gchar*)g_new(int8_t*, n_properties);
        break;
    case 16:
        property_data = (gchar*)g_new(int16_t*, n_properties);
        break;
    case 32:
    default:
        property_data = (gchar*)g_new(int32_t*, n_properties);
        break;
    }

    for (i = 0; i < n_properties; i++) {
        switch (format_type) {
        case 8:
            *(((int8_t*)property_data) + i) = properties[i];
            break;
        case 16:
            *(((int16_t*)property_data) + i) = properties[i];
            break;
        case 32:
        default:
            *(((int32_t*)property_data) + i) = properties[i];
            break;
        }
    }

    gdk_error_trap_push();
    XChangeDeviceProperty(GDK_DISPLAY(),
                          device, property_atom,
                          XA_INTEGER, format_type, PropModeReplace,
                          (unsigned char*)property_data, n_properties);
    gdk_error_trap_pop();

    g_free(property_data);

    return TRUE;
}

static Atom
get_atom (GpdsXInput *xinput, const gchar *property_name, GError **error)
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
            XFree(name);
            break;
        }
        XFree(name);
    }
    XFree(properties);

    return found_atom;
}

static gboolean
get_int_properties (GpdsXInput *xinput,
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
    Status status;

    g_return_val_if_fail(GPDS_IS_XINPUT(xinput), FALSE);

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    atom = get_atom(xinput, property_name, error);
    if (atom < 0)
        return FALSE;

    gdk_error_trap_push();
    status =  XGetDeviceProperty(GDK_DISPLAY(), device, atom, 0, 1000, False,
                                 XA_INTEGER, &actual_type, &actual_format,
                                 n_values, &bytes_after, &data);
    gdk_error_trap_pop();

    if (status != Success)
        return FALSE;

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
gpds_xinput_get_int_properties (GpdsXInput *xinput,
                                const gchar *property_name,
                                GError **error,
                                gint **values,
                                gulong *n_values)
{
    XDevice *device;

    g_return_val_if_fail(GPDS_IS_XINPUT(xinput), FALSE);

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    return get_int_properties(xinput, property_name, error, values, n_values);
}

gboolean
gpds_xinput_set_float_properties (GpdsXInput *xinput,
                                  const gchar *property_name,
                                  GError **error,
                                  gdouble *properties,
                                  guint n_properties)
{
    XDevice *device;
    Atom float_atom, property_atom;
    gint i;
    gfloat *property_data;

    g_return_val_if_fail(GPDS_IS_XINPUT(xinput), FALSE);

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    float_atom = gpds_xinput_utils_get_float_atom(error);
    if (float_atom == 0)
        return FALSE;

    property_atom = XInternAtom(GDK_DISPLAY(), property_name, False);

    property_data = g_new(gfloat, n_properties);
    for (i = 0; i < n_properties; i++)
        *(property_data + i) = (gfloat)properties[i];

    gdk_error_trap_push();
    XChangeDeviceProperty(GDK_DISPLAY(),
                          device, property_atom,
                          float_atom, 32, PropModeReplace,
                          (unsigned char*)property_data, n_properties);
    gdk_error_trap_pop();

    g_free(property_data);

    return TRUE;
}

gboolean
gpds_xinput_get_float_properties (GpdsXInput *xinput,
                                  const gchar *property_name,
                                  GError **error,
                                  gdouble **properties,
                                  gulong *n_properties)
{
    XDevice *device;
    Atom property_atom, float_atom;
    Atom actual_type;
    int actual_format;
    unsigned long bytes_after;
    unsigned char *data, *data_position;
    gulong i;
    gdouble *double_values;
    Status status;

    g_return_val_if_fail(GPDS_IS_XINPUT(xinput), FALSE);

    device = get_device(xinput, error);
    if (!device)
        return FALSE;

    float_atom = gpds_xinput_utils_get_float_atom(error);
    if (float_atom == 0)
        return FALSE;

    property_atom = get_atom(xinput, property_name, error);
    if (property_atom < 0)
        return FALSE;

    gdk_error_trap_push();
    status =  XGetDeviceProperty(GDK_DISPLAY(), device, property_atom, 0, 1000, False,
                                 float_atom, &actual_type, &actual_format,
                                 n_properties, &bytes_after, &data);
    gdk_error_trap_pop();

    if (status != Success)
        return FALSE;

    if (actual_type != float_atom) {
        XFree(data);
        return FALSE;
    }

    data_position = data;
    double_values = g_new(gdouble, *n_properties);

    for (i = 0; i < *n_properties; i++) {
        double_values[i] = *((float*)data_position);
        data_position += actual_format / 8;
    }

    *properties = double_values;
    XFree(data);

    return TRUE;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
