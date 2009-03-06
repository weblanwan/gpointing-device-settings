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

#ifndef __GPDS_XINPUT_UTILS_H__
#define __GPDS_XINPUT_UTILS_H__

#include <glib.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>

G_BEGIN_DECLS

#define GPDS_XINPUT_UTILS_ERROR           (gpds_xinput_utils_error_quark())

typedef enum
{
    GPDS_XINPUT_UTILS_ERROR_NO_DEVICE,
    GPDS_XINPUT_UTILS_ERROR_UNABLE_TO_OPEN_DEVICE,
    GPDS_XINPUT_UTILS_ERROR_NO_FLOAT_ATOM
} GpdsXInputUtilsError;

GQuark       gpds_xinput_utils_error_quark              (void);
XDeviceInfo *gpds_xinput_utils_get_device_info          (const gchar *device_name);
XDevice     *gpds_xinput_utils_open_device              (const gchar *device_name, GError **error);
Atom         gpds_xinput_utils_get_float_atom           (GError **error);
gboolean     gpds_xinput_utils_exist_device             (const gchar *device_name);
GList       *gpds_xinput_utils_get_pointer_device_names (void);

G_END_DECLS

#endif /* __GPDS_XINPUT_UTILS_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/

