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

#include "gsd-mouse-extension-manager.h"
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>
#include <gpds-xinput-utils.h>
#include <gpds-gconf.h>

#include "gpds-mouse-definitions.h"
#include "gpds-mouse-xinput.h"

G_DEFINE_TYPE (GsdMouseExtensionManager, gsd_mouse_extension_manager, GSD_TYPE_POINTING_DEVICE_MANAGER)

static void _gconf_client_notify (GsdPointingDeviceManager *manager,
                                  GConfClient *client,
                                  guint cnxn_id,
                                  GConfEntry *entry);

static void
gsd_mouse_extension_manager_init (GsdMouseExtensionManager *manager)
{
}

static void
gsd_mouse_extension_manager_class_init (GsdMouseExtensionManagerClass *klass)
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
    key = gpds_gconf_get_key_from_path(gconf_entry_get_key(entry));

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

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
