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

#ifndef __GPDS_UI_H__
#define __GPDS_UI_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPDS_UI_ERROR           (gpds_ui_error_quark())

#define GPDS_TYPE_UI            (gpds_ui_get_type ())
#define GPDS_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_UI, GpdsUI))
#define GPDS_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_UI, GpdsUIClass))
#define GPDS_IS_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_UI))
#define GPDS_IS_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_UI))
#define GPDS_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_UI, GpdsUIClass))

typedef enum
{
    GPDS_UI_ERROR_NO_UI_FILE,
    GPDS_UI_ERROR_NO_WIDGET
} GpdsUIError;

typedef struct _GpdsUI GpdsUI;
typedef struct _GpdsUIClass GpdsUIClass;

struct _GpdsUI
{
    GObject parent;
};

struct _GpdsUIClass
{
    GObjectClass parent_class;

    gboolean   (*is_available)          (GpdsUI *ui, GError **error);
    gboolean   (*build)                 (GpdsUI *ui, GError **error);
    GtkWidget *(*get_content_widget)    (GpdsUI *ui, GError **error);
    GdkPixbuf *(*get_icon_pixbuf)       (GpdsUI *ui, GError **error);
};

gboolean     gpds_uis_load                (void);
gboolean     gpds_uis_unload              (void);
GList       *gpds_uis_get_names           (void);

GQuark       gpds_ui_error_quark          (void);
GType        gpds_ui_get_type             (void) G_GNUC_CONST;
GpdsUI      *gpds_ui_new                  (const gchar *type_name,
                                           const gchar *first_property,
                                           ...);
gboolean     gpds_ui_is_available         (GpdsUI *ui, GError **error);
gboolean     gpds_ui_build                (GpdsUI *ui, GError **error);
GtkWidget   *gpds_ui_get_content_widget   (GpdsUI *ui, GError **error);
GdkPixbuf   *gpds_ui_get_icon_pixbuf      (GpdsUI *ui, GError **error);
GtkBuilder  *gpds_ui_get_builder          (GpdsUI *ui);
GObject     *gpds_ui_get_ui_object_by_name(GpdsUI *ui, const gchar *name);
const gchar *gpds_ui_get_device_name      (GpdsUI *ui);
void         gpds_ui_set_gconf_bool       (GpdsUI *ui,
                                           const gchar *key,
                                           gboolean value);
gboolean     gpds_ui_get_gconf_bool       (GpdsUI *ui,
                                           const gchar *key,
                                           gboolean *value);
void         gpds_ui_set_gconf_int        (GpdsUI *ui,
                                           const gchar *key,
                                           gboolean value);
gboolean     gpds_ui_get_gconf_int        (GpdsUI *ui,
                                           const gchar *key,
                                           gboolean *value);
void         gpds_ui_set_gconf_string     (GpdsUI *ui,
                                           const gchar *key,
                                           const gchar *value);
gboolean     gpds_ui_get_gconf_string     (GpdsUI *ui,
                                           const gchar *key,
                                           gchar **value);

G_END_DECLS

#endif /* __GPDS_UI_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
