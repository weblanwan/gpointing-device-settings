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

#include "gpds-xinput-utils.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <string.h>
#include "gpds-xinput-pointer-info.h"

GQuark
gpds_xinput_utils_error_quark (void)
{
    return g_quark_from_static_string("gpds-xinput-utils-error-quark");
}

XDeviceInfo *
gpds_xinput_utils_get_device_info (const gchar *device_name)
{
    XDeviceInfo *device_infos;
    gint i, n_device_infos;

    device_infos = XListInputDevices(GDK_DISPLAY(), &n_device_infos);

    for (i = 0; i < n_device_infos; i++) {
        if (device_infos[i].use != IsXExtensionPointer)
            continue;
        if (!strcmp(device_infos[i].name, device_name))
            return &device_infos[i];
    }

    XFreeDeviceList(device_infos);

    return NULL;
}

XDevice *
gpds_xinput_utils_open_device (const gchar *device_name, GError **error)
{
    XDeviceInfo *device_info;
    XDevice *device;

    device_info = gpds_xinput_utils_get_device_info(device_name);
    if (!device_info) {
        g_set_error(error,
                    GPDS_XINPUT_UTILS_ERROR,
                    GPDS_XINPUT_UTILS_ERROR_NO_DEVICE,
                    _("No %s found."), device_name);
        return NULL;
    }

    gdk_error_trap_push();
    device = XOpenDevice(GDK_DISPLAY(), device_info->id);
    gdk_error_trap_pop();
    if (!device) {
        g_set_error(error,
                    GPDS_XINPUT_UTILS_ERROR,
                    GPDS_XINPUT_UTILS_ERROR_NO_DEVICE,
                    _("Could not open %s device."), device_name);
        return NULL;
    }

    return device;
}

Atom
gpds_xinput_utils_get_float_atom (GError **error)
{
    Atom float_atom;

    float_atom = XInternAtom(GDK_DISPLAY(), "FLOAT", False);
    if (float_atom == 0) {
        g_set_error(error,
                    GPDS_XINPUT_UTILS_ERROR,
                    GPDS_XINPUT_UTILS_ERROR_NO_FLOAT_ATOM,
                    _("No float atom in XServer"));
    }

    return float_atom;
}

gboolean
gpds_xinput_utils_exist_device (const gchar *device_name)
{
    return gpds_xinput_utils_get_device_info(device_name) ? TRUE : FALSE;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
