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
#include <gpds-xinput-utils.h>

#include "gpds-touchpad-definitions.h"
#include "gpds-touchpad-xinput.h"

G_DEFINE_TYPE (GsdTouchpadManager, gsd_touchpad_manager, GSD_TYPE_POINTING_DEVICE_MANAGER)

static void _gconf_client_notify (GsdPointingDeviceManager *manager,
                                  GConfClient *client,
                                  guint cnxn_id,
                                  GConfEntry *entry);

static void
gsd_touchpad_manager_init (GsdTouchpadManager *manager)
{
}

static void
gsd_touchpad_manager_class_init (GsdTouchpadManagerClass *klass)
{
    GsdPointingDeviceManagerClass *manager_class = GSD_POINTING_DEVICE_MANAGER_CLASS(klass);

    manager_class->gconf_client_notify = _gconf_client_notify;
}

static void
_gconf_client_notify (GsdPointingDeviceManager *manager,
                      GConfClient *client,
                      guint cnxn_id,
                      GConfEntry *entry)
{
{
    GConfValue *value;
    const gchar *key;
    GpdsXInput *xinput;
    gint properties[4];
    const gchar *device_name;

    device_name = gsd_pointing_device_manager_get_device_name(manager);
    if (!device_name)
        return;

    if (!gpds_xinput_utils_exist_device(device_name))
        return;

    xinput = gpds_xinput_new(device_name);

    value = gconf_entry_get_value(entry);
    key = gconf_entry_get_key(entry);

    switch (value->type) {
    case GCONF_VALUE_BOOL:
        if (!strcmp(key, GPDS_TOUCHPAD_TAP_FAST_TAP_KEY)) {
            properties[0] = gconf_value_get_bool(value) ? 1 : 0;
            gpds_xinput_set_int_properties(xinput,
                                           gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_TAP_FAST_TAP),
                                           gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_TAP_FAST_TAP),
                                           NULL,
                                           properties,
                                           1);
        } else  if (!strcmp(key, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY)) {
            properties[0] = gconf_value_get_bool(value) ? 1 : 0;
            gpds_xinput_set_int_properties(xinput,
                                           gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_CIRCULAR_SCROLLING),
                                           gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_CIRCULAR_SCROLLING),
                                           NULL,
                                           properties,
                                           1);
        } else  if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY) ||
                    !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY)) {
            gboolean enable;
            enable = gconf_client_get_bool(client,
                                           GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY,
                                           NULL);
            properties[0] = enable ? 1 : 0;
            enable = gconf_client_get_bool(client,
                                           GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY,
                                           NULL);
            properties[1] = enable ? 1 : 0;

            gpds_xinput_set_int_properties(xinput,
                                           gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_EDGE_SCROLLING),
                                           gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_EDGE_SCROLLING),
                                           NULL,
                                           properties,
                                           2);
        }
        break;
    case GCONF_VALUE_INT:
        if (!strcmp(key, GPDS_TOUCHPAD_OFF_KEY)) {
            properties[0] = gconf_value_get_int(value);
            gpds_xinput_set_int_properties(xinput,
                                           gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_OFF),
                                           gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_OFF),
                                           NULL,
                                           properties,
                                           1);
        } else if (!strcmp(key, GPDS_TOUCHPAD_TAP_TIME_KEY)) {
            properties[0] = gconf_value_get_int(value);
            gpds_xinput_set_int_properties(xinput,
                                           gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_TAP_TIME),
                                           gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_TAP_TIME),
                                           NULL,
                                           properties,
                                           1);
        } else if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY)) {
            properties[0] =
                gconf_client_get_int(client,
                                     GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY,
                                     NULL);
            properties[1] =
                gconf_client_get_int(client,
                                     GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY,
                                     NULL);
            gpds_xinput_set_int_properties(xinput,
                                           gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_SCROLLING_DISTANCE),
                                           gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_SCROLLING_DISTANCE),
                                           NULL,
                                           properties,
                                           2);
        }
        break;
    default:
        break;
    }

    g_object_unref(xinput);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
