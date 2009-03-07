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

#include "gpds-xinput-pointer-info.h"

#include <gdk/gdkx.h>
#include "gpds-xinput-utils.h"

GpdsXInputPointerInfo *
gpds_xinput_pointer_info_new (const gchar *name, const gchar *type_name)
{
    GpdsXInputPointerInfo *info;

    info = g_new0(GpdsXInputPointerInfo, 1);
    info->name = g_strdup(name);
    info->type_name = g_strdup(type_name);

    return info;
}

void
gpds_xinput_pointer_info_free (GpdsXInputPointerInfo *info)
{
    g_free(info->name);
    g_free(info->type_name);
    g_free(info);
}

GList *
gpds_xinput_utils_collect_pointer_infos (void)
{
    GList *device_names = NULL;
    XDeviceInfo *device_infos;
    gint i, n_device_infos;

    device_infos = XListInputDevices(GDK_DISPLAY(), &n_device_infos);

    for (i = 0; i < n_device_infos; i++) {
        GpdsXInputPointerInfo *info;

        if (device_infos[i].use != IsXExtensionPointer)
            continue;
        info = gpds_xinput_pointer_info_new(device_infos[i].name,
                                            XGetAtomName(GDK_DISPLAY(), device_infos[i].type));

        device_names = g_list_append(device_names, info);
    }

    XFreeDeviceList(device_infos);

    return device_names;
}

const gchar *
gpds_xinput_pointer_info_get_name (GpdsXInputPointerInfo *info)
{
    return info->name;
}

const gchar *
gpds_xinput_pointer_info_get_type_name (GpdsXInputPointerInfo *info)
{
    return info->type_name;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
