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

#ifndef __G_XINPUT_H__
#define __G_XINPUT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define G_XINPUT_ERROR           (g_xinput_error_quark())

#define G_TYPE_XINPUT            (g_xinput_get_type ())
#define G_XINPUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_XINPUT, GXInput))
#define G_XINPUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_XINPUT, GXInputClass))
#define G_IS_XINPUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_XINPUT))
#define G_IS_XINPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_XINPUT))
#define G_XINPUT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), G_TYPE_XINPUT, GXInputClass))

typedef enum
{
    G_XINPUT_ERROR_NO_DEVICE,
    G_XINPUT_ERROR_UNABLE_TO_OPEN_DEVICE
} GXInputError;

typedef struct _GXInput GXInput;
typedef struct _GXInputClass GXInputClass;

struct _GXInput
{
    GObject parent;
};

struct _GXInputClass
{
    GObjectClass parent_class;
};

GQuark       g_xinput_error_quark     (void);

GType        g_xinput_get_type        (void) G_GNUC_CONST;

gboolean     g_xinput_exist_device    (const gchar *device_name);

GXInput     *g_xinput_new             (const gchar *device_name);

gboolean     g_xinput_set_property    (GXInput *xinput,
                                       const gchar *property_name,
                                       GError **error,
                                       gint first_property_value,
                                       ...) G_GNUC_NULL_TERMINATED;
gboolean     g_xinput_get_property    (GXInput *xinput,
                                       const gchar *property_name,
                                       GError **error,
                                       gint **values,
                                       gulong *n_values);

G_END_DECLS

#endif /* __G_XINPUT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/

