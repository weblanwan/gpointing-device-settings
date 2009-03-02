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

#include "gsd-touchpad-manager.h"
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>

#include "gpds-touchpad-definitions.h"

#define GSD_TOUCHPAD_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GSD_TYPE_TOUCHPAD_MANAGER, GsdTouchpadManagerPrivate))

typedef struct _GsdTouchpadManagerPrivate  GsdTouchpadManagerPrivate;
struct _GsdTouchpadManagerPrivate
{
    GConfClient *gconf;
    guint notify_id;
};

G_DEFINE_TYPE (GsdTouchpadManager, gsd_touchpad_manager, G_TYPE_OBJECT)

static gpointer manager_object = NULL;

static void
gsd_touchpad_manager_init (GsdTouchpadManager *manager)
{
    GsdTouchpadManagerPrivate *priv;

    priv = GSD_TOUCHPAD_MANAGER_GET_PRIVATE(manager);

    priv->gconf = NULL;
    priv->notify_id = 0;
}

static void
gsd_touchpad_manager_class_init (GsdTouchpadManagerClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(gobject_class, sizeof(GsdTouchpadManagerPrivate));
}

GsdTouchpadManager *
gsd_touchpad_manager_new (void)
{
    if (manager_object != NULL) {
        g_object_ref(manager_object);
    } else {
        manager_object = g_object_new(GSD_TYPE_TOUCHPAD_MANAGER, NULL);
        g_object_add_weak_pointer(manager_object, (gpointer *)&manager_object);
    }

    return GSD_TOUCHPAD_MANAGER(manager_object);
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

    if (!gpds_xinput_exist_device(GPDS_TOUCHPAD_DEVICE_NAME))
        return;

    xinput = gpds_xinput_new(GPDS_TOUCHPAD_DEVICE_NAME);

    value = gconf_entry_get_value(entry);
    key = gconf_entry_get_key(entry);

    switch (value->type) {
    case GCONF_VALUE_BOOL:
        if (!strcmp(key, GPDS_TOUCHPAD_TAP_FAST_TAP_KEY)) {
            gpds_xinput_set_property(xinput,
                                     GPDS_TOUCHPAD_TAP_FAST_TAP,
                                     NULL,
                                     gconf_value_get_bool(value),
                                     NULL);
        } else  if (!strcmp(key, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY)) {
            gpds_xinput_set_property(xinput,
                                     GPDS_TOUCHPAD_CIRCULAR_SCROLLING,
                                     NULL,
                                     gconf_value_get_bool(value),
                                     NULL);
        } else  if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY) ||
                    !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY)) {
            gboolean enable_vertical, enable_horizontal;
            enable_vertical = gconf_client_get_bool(client,
                                                    GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY,
                                                    NULL);
            enable_horizontal = gconf_client_get_bool(client,
                                                      GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY,
                                                      NULL);

            gpds_xinput_set_property(xinput,
                                     GPDS_TOUCHPAD_EDGE_SCROLLING,
                                     NULL,
                                     enable_vertical ? 1 : 0,
                                     enable_horizontal ? 1 : 0,
                                     NULL);
        }
        break;
    case GCONF_VALUE_INT:
        if (!strcmp(key, GPDS_TOUCHPAD_TAP_TIME_KEY)) {
            gpds_xinput_set_property(xinput,
                                     GPDS_TOUCHPAD_TAP_TIME,
                                     NULL,
                                     gconf_value_get_int(value),
                                     NULL);
        } else if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY)) {
            gint vertical_scrolling_distance;
            gint horizontal_scrolling_distance;

            vertical_scrolling_distance =
                gconf_client_get_int(client,
                                     GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY,
                                     NULL);
            horizontal_scrolling_distance =
                gconf_client_get_int(client,
                                     GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY,
                                     NULL);
            gpds_xinput_set_property(xinput,
                                     GPDS_TOUCHPAD_SCROLLING_DISTANCE,
                                     NULL,
                                     vertical_scrolling_distance,
                                     horizontal_scrolling_distance,
                                     NULL);
        }
        break;
    default:
        break;
    }

    g_object_unref(xinput);
}

gboolean
gsd_touchpad_manager_start (GsdTouchpadManager *manager,
                               GError              **error)
{
    GsdTouchpadManagerPrivate *priv;

    priv = GSD_TOUCHPAD_MANAGER_GET_PRIVATE(manager);
    priv->gconf = gconf_client_get_default();

    gconf_client_add_dir(priv->gconf,
                         GPDS_TOUCHPAD_GCONF_DIR,
                         GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
    priv->notify_id = gconf_client_notify_add(priv->gconf,
                                              GPDS_TOUCHPAD_GCONF_DIR,
                                              cb_gconf_client_notify,
                                              manager,
                                              NULL,
                                              NULL);

    return TRUE;
}

void
gsd_touchpad_manager_stop (GsdTouchpadManager *manager)
{
    GsdTouchpadManagerPrivate *priv;

    priv = GSD_TOUCHPAD_MANAGER_GET_PRIVATE(manager);

    if (priv->notify_id) {
        gconf_client_remove_dir(priv->gconf,
                                GPDS_TOUCHPAD_GCONF_DIR,
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