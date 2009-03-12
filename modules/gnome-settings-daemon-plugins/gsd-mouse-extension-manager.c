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

static gboolean start                (GsdPointingDeviceManager *manager,
                                      GError **error);
static void     stop                 (GsdPointingDeviceManager *manager);
static void     _gconf_client_notify (GsdPointingDeviceManager *manager,
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

    manager_class->start               = start;
    manager_class->stop                = stop;
    manager_class->gconf_client_notify = _gconf_client_notify;
}

DEFINE_SET_BOOLEAN_FUNCTION (wheel_emulation, GPDS_MOUSE_WHEEL_EMULATION)
DEFINE_SET_BOOLEAN_FUNCTION (middle_button_emulation, GPDS_MOUSE_MIDDLE_BUTTON_EMULATION)
DEFINE_SET_INT_FUNCTION (middle_button_timeout, GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT)
DEFINE_SET_INT_FUNCTION (wheel_emulation_button, GPDS_MOUSE_WHEEL_EMULATION_BUTTON)
DEFINE_SET_INT_FUNCTION (wheel_emulation_timeout, GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT)
DEFINE_SET_INT_FUNCTION (wheel_emulation_inertia, GPDS_MOUSE_WHEEL_EMULATION_INERTIA)

static void
set_horizontal_and_vertical_scroll (GsdPointingDeviceManager *manager,
                                    GpdsXInput *xinput,
                                    GConfClient *gconf)
{
    gboolean y_enable, x_enable;
    gint properties[4];

    if (!gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                       gconf,
                                                       GPDS_MOUSE_WHEEL_EMULATION_Y_AXIS_KEY,
                                                       &y_enable)) {
        return;
    }
    if (!gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                       gconf,
                                                       GPDS_MOUSE_WHEEL_EMULATION_X_AXIS_KEY,
                                                       &x_enable)) {
        return;
    }

    if (y_enable) {
        properties[0] = 6;
        properties[1] = 7;
    } else {
        properties[0] = 0;
        properties[1] = 0;
    }
    if (x_enable) {
        properties[2] = 4;
        properties[3] = 5;
    } else {
        properties[2] = 0;
        properties[3] = 0;
    }
    gpds_xinput_set_int_properties(xinput,
                                   GPDS_MOUSE_WHEEL_EMULATION_AXES,
                                   NULL,
                                   properties,
                                   4);
}

static gboolean
start_manager (GsdPointingDeviceManager *manager)
{
    GpdsXInput *xinput;
    GConfClient *gconf;

    xinput = gsd_pointing_device_manager_get_xinput(manager);
    if (!xinput)
        return FALSE;

    gconf = gconf_client_get_default();
    if (!gconf) {
        g_object_unref(xinput);
        return FALSE;
    }

    gpds_mouse_xinput_setup_property_entries(xinput);
    set_middle_button_emulation(manager, xinput, gconf);
    set_wheel_emulation(manager, xinput, gconf);
    set_middle_button_timeout(manager, xinput, gconf);
    set_wheel_emulation_button(manager, xinput, gconf);
    set_wheel_emulation_timeout(manager, xinput, gconf);
    set_wheel_emulation_inertia(manager, xinput, gconf);
    set_horizontal_and_vertical_scroll(manager, xinput, gconf);

    g_object_unref(gconf);
    g_object_unref(xinput);

    return FALSE;
}

static gboolean
start (GsdPointingDeviceManager *manager, GError **error)
{
    g_idle_add((GSourceFunc)start_manager, manager);

    return TRUE;
}

static void
stop (GsdPointingDeviceManager *manager)
{
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

    xinput = gsd_pointing_device_manager_get_xinput(manager);
    if (!xinput)
        return;

    gpds_mouse_xinput_setup_property_entries(xinput);
    value = gconf_entry_get_value(entry);
    key = gpds_gconf_get_key_from_path(gconf_entry_get_key(entry));

    switch (value->type) {
    case GCONF_VALUE_BOOL:
        if (!strcmp(key, GPDS_MOUSE_MIDDLE_BUTTON_EMULATION_KEY)) {
            set_middle_button_emulation(manager, xinput, client);
        } else  if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_KEY)) {
            set_wheel_emulation(manager, xinput, client);
        } else  if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_X_AXIS_KEY) ||
                  !strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_Y_AXIS_KEY)) {
            set_horizontal_and_vertical_scroll(manager, xinput, client);
        }
        break;
    case GCONF_VALUE_INT:
        if (!strcmp(key, GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT_KEY))
            set_middle_button_timeout(manager, xinput, client);
        else if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT_KEY))
            set_wheel_emulation_timeout(manager, xinput, client);
        else if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_INERTIA_KEY))
            set_wheel_emulation_inertia(manager, xinput, client);
        else if (!strcmp(key, GPDS_MOUSE_WHEEL_EMULATION_BUTTON_KEY))
            set_wheel_emulation_button(manager, xinput, client);
        break;
    default:
        break;
    }

    g_object_unref(xinput);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
