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

#ifndef __GPDS_XINPUT_H__
#define __GPDS_XINPUT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GPDS_TYPE_XINPUT            (gpds_xinput_get_type ())
#define GPDS_XINPUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_XINPUT, GpdsXInput))
#define GPDS_XINPUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_XINPUT, GpdsXInputClass))
#define GPDS_IS_XINPUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_XINPUT))
#define GPDS_IS_XINPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_XINPUT))
#define GPDS_XINPUT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_XINPUT, GpdsXInputClass))

#define GPDS_XINPUT_ERROR           (gpds_xinput_error_quark())

typedef enum
{
    GPDS_XINPUT_ERROR_NO_REGISTERED_PROPERTY,
    GPDS_XINPUT_ERROR_X_ERROR
} GpdsXInputError;

typedef struct _GpdsXInputPropertyEntry GpdsXInputPropertyEntry;
struct _GpdsXInputPropertyEntry
{
    gint property_enum;
    const gchar *name;
    GType property_type; /* G_TYPE_INT or G_TYPE_FLOAT */
    gint format_type;
    gint max_value_count;
};

typedef struct _GpdsXInput GpdsXInput;
typedef struct _GpdsXInputClass GpdsXInputClass;

struct _GpdsXInput
{
    GObject parent;
};

struct _GpdsXInputClass
{
    GObjectClass parent_class;
};

GQuark       gpds_xinput_error_quark          (void);
GType        gpds_xinput_get_type             (void) G_GNUC_CONST;

GpdsXInput  *gpds_xinput_new                  (const gchar *device_name);
const gchar *gpds_xinput_get_device_name      (GpdsXInput *xinput);
void         gpds_xinput_register_property_entries
                                              (GpdsXInput *xinput,
                                               const GpdsXInputPropertyEntry *entries,
                                               guint n_entries);
gboolean     gpds_xinput_set_int_properties   (GpdsXInput *xinput,
                                               gint property_enum,
                                               GError **error,
                                               gint *properties,
                                               guint n_properties);
gboolean     gpds_xinput_set_int_properties_by_name_with_format_type
                                              (GpdsXInput *xinput,
                                               const gchar *property_name,
                                               gint format_type,
                                               GError **error,
                                               gint *properties,
                                               guint n_properties);
gboolean     gpds_xinput_get_int_properties
                                              (GpdsXInput *xinput,
                                               gint property_enum,
                                               GError **error,
                                               gint **values,
                                               gulong *n_values);
gboolean     gpds_xinput_get_int_properties_by_name
                                              (GpdsXInput *xinput,
                                               const gchar *property_name,
                                               GError **error,
                                               gint **values,
                                               gulong *n_values);
gboolean     gpds_xinput_set_float_properties (GpdsXInput *xinput,
                                               gint property_enum,
                                               GError **error,
                                               gdouble *properties,
                                               guint n_properties);
gboolean     gpds_xinput_set_float_properties_by_name
                                              (GpdsXInput *xinput,
                                               const gchar *property_name,
                                               GError **error,
                                               gdouble *properties,
                                               guint n_properties);
gboolean     gpds_xinput_get_float_properties (GpdsXInput *xinput,
                                               gint property_enum,
                                               GError **error,
                                               gdouble **properties,
                                               gulong *n_properties);
gboolean     gpds_xinput_get_float_properties_by_name
                                              (GpdsXInput *xinput,
                                               const gchar *property_name,
                                               GError **error,
                                               gdouble **properties,
                                               gulong *n_properties);


G_END_DECLS

#endif /* __GPDS_XINPUT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/

