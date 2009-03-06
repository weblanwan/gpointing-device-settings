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

#include "gsd-trackpoint-manager.h"
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>
#include <gpds-xinput-utils.h>

#include "gpds-mouse-definitions.h"
#include "gpds-mouse-xinput.h"

#define GSD_TRACK_POINT_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GSD_TYPE_TRACK_POINT_MANAGER, GsdTrackPointManagerPrivate))

typedef struct _GsdTrackPointManagerPrivate  GsdTrackPointManagerPrivate;
struct _GsdTrackPointManagerPrivate
{
    GConfClient *gconf;
    guint notify_id;
};

G_DEFINE_TYPE (GsdTrackPointManager, gsd_track_point_manager, G_TYPE_OBJECT)

static gpointer manager_object = NULL;

static void
gsd_track_point_manager_init (GsdTrackPointManager *manager)
{
    GsdTrackPointManagerPrivate *priv;

    priv = GSD_TRACK_POINT_MANAGER_GET_PRIVATE(manager);

    priv->gconf = NULL;
    priv->notify_id = 0;
}

static void
gsd_track_point_manager_class_init (GsdTrackPointManagerClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(gobject_class, sizeof(GsdTrackPointManagerPrivate));
}

GsdTrackPointManager *
gsd_track_point_manager_new (void)
{
    if (manager_object != NULL) {
        g_object_ref(manager_object);
    } else {
        manager_object = g_object_new(GSD_TYPE_TRACK_POINT_MANAGER, NULL);
        g_object_add_weak_pointer(manager_object, (gpointer *)&manager_object);
    }

    return GSD_TRACK_POINT_MANAGER(manager_object);
}

static void
cb_gconf_client_notify (GConfClient *client,
                        guint cnxn_id,
                        GConfEntry *entry,
                        gpointer user_data)
{
    GConfValue *value;
    const gchar *key;
    GpdsXInput *xinput;
    gint properties[4];
    const gchar *device_name;

    device_name = gpds_mouse_xinput_find_device_name();
    if (!device_name)
        return;

    if (!gpds_xinput_utils_exist_device(device_name))
        return;

    xinput = gpds_xinput_new(device_name);

    value = gconf_entry_get_value(entry);
    key = gconf_entry_get_key(entry);

    switch (value->type) {
    case GCONF_VALUE_BOOL:
        if (!strcmp(key, GPDS_MOUSE_MIDDLE_BUTTON_EMULATION_KEY)) {
            properties[0] = gconf_value_get_bool(value) ? 1 : 0;
            gpds_xinput_set_int_properties(xinput,
                                           gpds_mouse_xinput_get_name(GPDS_MOUSE_MIDDLE_BUTTON_EMULATION),
                                           gpds_mouse_xinput_get_format_type(GPDS_MOUSE_MIDDLE_BUTTON_EMULATION),
                                           NULL,
                                           properties,
                                           1);
        } else  if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_KEY)) {
            properties[0] = gconf_value_get_bool(value) ? 1 : 0;
            gpds_xinput_set_int_properties(xinput,
                                           gpds_mouse_xinput_get_name(GPDS_MOUSE_WHEEL_EMULATION),
                                           gpds_mouse_xinput_get_format_type(GPDS_MOUSE_WHEEL_EMULATION),
                                           NULL,
                                           properties,
                                           1);
        } else  if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_X_AXIS_KEY) ||
                    !strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_Y_AXIS_KEY)) {
            gboolean enable;
            enable = gconf_client_get_bool(client,
                                           GPDS_MOUSE_WHEEL_EMULATION_Y_AXIS_KEY,
                                           NULL);
            if (enable) {
                properties[0] = 6;
                properties[1] = 7;
            } else {
                properties[0] = 0;
                properties[1] = 0;
            }

            enable = gconf_client_get_bool(client,
                                           GPDS_MOUSE_WHEEL_EMULATION_X_AXIS_KEY,
                                           NULL);
            if (enable) {
                properties[2] = 4;
                properties[3] = 5;
            } else {
                properties[2] = 0;
                properties[3] = 0;
            }
            gpds_xinput_set_int_properties(xinput,
                                           gpds_mouse_xinput_get_name(GPDS_MOUSE_WHEEL_EMULATION_AXES),
                                           gpds_mouse_xinput_get_format_type(GPDS_MOUSE_WHEEL_EMULATION_AXES),
                                           NULL,
                                           properties,
                                           4);
        }
        break;
    case GCONF_VALUE_INT:
        properties[0] = gconf_value_get_int(value);
        if (!strcmp(key, GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT_KEY)) {
            gpds_xinput_set_int_properties(xinput,
                                           gpds_mouse_xinput_get_name(GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT),
                                           gpds_mouse_xinput_get_format_type(GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT),
                                           NULL,
                                           properties,
                                           1);
        } else if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT_KEY)) {
            gpds_xinput_set_int_properties(xinput,
                                           gpds_mouse_xinput_get_name(GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT),
                                           gpds_mouse_xinput_get_format_type(GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT),
                                           NULL,
                                           properties,
                                           1);
        } else if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_INERTIA_KEY)) {
            gpds_xinput_set_int_properties(xinput,
                                           gpds_mouse_xinput_get_name(GPDS_MOUSE_WHEEL_EMULATION_INERTIA),
                                           gpds_mouse_xinput_get_format_type(GPDS_MOUSE_WHEEL_EMULATION_INERTIA),
                                           NULL,
                                           properties,
                                           1);
        }
        break;
    default:
        break;
    }

    g_object_unref(xinput);
}

gboolean
gsd_track_point_manager_start (GsdTrackPointManager *manager,
                               GError              **error)
{
    GsdTrackPointManagerPrivate *priv;
    gchar *gconf_dir;
    const gchar *device_name;
    gchar *escaped_device_name;

    priv = GSD_TRACK_POINT_MANAGER_GET_PRIVATE(manager);
    priv->gconf = gconf_client_get_default();

    device_name = gpds_mouse_xinput_find_device_name();
    if (!device_name)
        return FALSE;

    escaped_device_name = gconf_escape_key(device_name, -1);
    gconf_dir = g_strdup_printf("%s/%s",
                                GPDS_MOUSE_GCONF_DIR,
                                escaped_device_name);
    gconf_client_add_dir(priv->gconf,
                         gconf_dir,
                         GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
    priv->notify_id = gconf_client_notify_add(priv->gconf,
                                              gconf_dir,
                                              cb_gconf_client_notify,
                                              manager,
                                              NULL,
                                              NULL);
    g_free(escaped_device_name);
    g_free(gconf_dir);

    return TRUE;
}

void
gsd_track_point_manager_stop (GsdTrackPointManager *manager)
{
    GsdTrackPointManagerPrivate *priv;

    priv = GSD_TRACK_POINT_MANAGER_GET_PRIVATE(manager);

    if (priv->notify_id) {
        gconf_client_remove_dir(priv->gconf,
                                GPDS_MOUSE_GCONF_DIR,
                                NULL);
        gconf_client_notify_remove(priv->gconf, priv->notify_id);
        priv->notify_id = 0;
    }

    if (priv->gconf) {
        g_object_unref(priv->gconf);
        priv->gconf = NULL;
    }
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
