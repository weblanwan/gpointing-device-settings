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

#ifndef __GSD_POINTING_DEVICE_MANAGER_H__
#define __GSD_POINTING_DEVICE_MANAGER_H__

#include <glib-object.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>

G_BEGIN_DECLS

#define GSD_TYPE_POINTING_DEVICE_MANAGER            (gsd_pointing_device_manager_get_type ())
#define GSD_POINTING_DEVICE_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSD_TYPE_POINTING_DEVICE_MANAGER, GsdPointingDeviceManager))
#define GSD_POINTING_DEVICE_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSD_TYPE_POINTING_DEVICE_MANAGER, GsdPointingDeviceManagerClass))
#define GSD_IS_POINTING_DEVICE_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSD_TYPE_POINTING_DEVICE_MANAGER))
#define GSD_IS_POINTING_DEVICE_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSD_TYPE_POINTING_DEVICE_MANAGER))
#define GSD_POINTING_DEVICE_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSD_TYPE_POINTING_DEVICE_MANAGER, GsdPointingDeviceManagerClass))

typedef struct _GsdPointingDeviceManager GsdPointingDeviceManager;
typedef struct _GsdPointingDeviceManagerClass GsdPointingDeviceManagerClass;

struct _GsdPointingDeviceManager
{
    GObject parent;
};

struct _GsdPointingDeviceManagerClass
{
    GObjectClass parent_class;

    gboolean (*start)               (GsdPointingDeviceManager *manager,
                                     GError                  **error);
    void     (*stop)                (GsdPointingDeviceManager *manager);
    void     (*gconf_client_notify) (GsdPointingDeviceManager *manager,
                                     GConfClient              *client,
                                     guint                     cnxn_id,
                                     GConfEntry               *entry);
}; 

GType gsd_pointing_device_manager_get_type (void) G_GNUC_CONST;

GsdPointingDeviceManager *gsd_pointing_device_manager_new   (const gchar              *device_type,
                                                             const gchar              *device_name);
const gchar              *gsd_pointing_device_manager_get_device_name
                                                            (GsdPointingDeviceManager *manager);
gboolean                  gsd_pointing_device_manager_start (GsdPointingDeviceManager *manager,
                                                             GError                  **error);
void                      gsd_pointing_device_manager_stop  (GsdPointingDeviceManager *manager);
GpdsXInput               *gsd_pointing_device_manager_get_xinput
                                                            (GsdPointingDeviceManager *manager);
gchar                    *gsd_pointing_device_manager_build_gconf_key 
                                                            (GsdPointingDeviceManager *manager,
                                                             const gchar *key);

G_END_DECLS

#endif /* __GSD_POINTING_DEVICE_MANAGER_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
