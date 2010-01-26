/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2010 Hiroyuki Ikezoe  <poincare@ikezoe.net>
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

#include "gsd-pointingstick-manager.h"
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>
#include <gpds-xinput-utils.h>
#include <gpds-gconf.h>

#include "gpds-pointingstick-definitions.h"
#include "gpds-pointingstick-xinput.h"

G_DEFINE_TYPE (GsdPointingStickManager, gsd_pointingstick_manager, GSD_TYPE_POINTING_DEVICE_MANAGER)

static gboolean _start               (GsdPointingDeviceManager *manager,
                                      GError **error);
static void     _stop                (GsdPointingDeviceManager *manager);
static void     _gconf_client_notify (GsdPointingDeviceManager *manager,
                                      GConfClient *client,
                                      guint cnxn_id,
                                      GConfEntry *entry);

static void
gsd_pointingstick_manager_init (GsdPointingStickManager *manager)
{
}

static void
gsd_pointingstick_manager_class_init (GsdPointingStickManagerClass *klass)
{
    GsdPointingDeviceManagerClass *manager_class = GSD_POINTING_DEVICE_MANAGER_CLASS(klass);

    manager_class->start               = _start;
    manager_class->stop                = _stop;
    manager_class->gconf_client_notify = _gconf_client_notify;
}

DEFINE_SET_BOOLEAN_FUNCTION (scrolling, GPDS_POINTINGSTICK_SCROLLING)
DEFINE_SET_BOOLEAN_FUNCTION (press_to_select, GPDS_POINTINGSTICK_PRESS_TO_SELECT)
DEFINE_SET_INT_FUNCTION (middle_button_timeout, GPDS_POINTINGSTICK_MIDDLE_BUTTON_TIMEOUT)
DEFINE_SET_INT_FUNCTION (sensitivity, GPDS_POINTINGSTICK_SENSITIVITY)
DEFINE_SET_INT_FUNCTION (press_to_select_threshold, GPDS_POINTINGSTICK_PRESS_TO_SELECT_THRESHOLD)

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

    gpds_pointingstick_xinput_setup_property_entries(xinput);
    set_sensitivity(manager, xinput, gconf);
    set_scrolling(manager, xinput, gconf);
    set_middle_button_timeout(manager, xinput, gconf);
    set_press_to_select(manager, xinput, gconf);
    set_press_to_select_threshold(manager, xinput, gconf);

    g_object_unref(gconf);
    g_object_unref(xinput);

    return FALSE;
}

static gboolean
_start (GsdPointingDeviceManager *manager, GError **error)
{
    g_idle_add((GSourceFunc)start_manager, manager);

    return TRUE;
}

static void
_stop (GsdPointingDeviceManager *manager)
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

    gpds_pointingstick_xinput_setup_property_entries(xinput);
    value = gconf_entry_get_value(entry);
    key = gpds_gconf_get_key_from_path(gconf_entry_get_key(entry));

    switch (value->type) {
    case GCONF_VALUE_BOOL:
        if (!strcmp(key, GPDS_POINTINGSTICK_SCROLLING_KEY)) {
            set_scrolling(manager, xinput, client);
        } else  if (!strcmp(key, GPDS_POINTINGSTICK_PRESS_TO_SELECT_KEY)) {
            set_press_to_select(manager, xinput, client);
        }
        break;
    case GCONF_VALUE_INT:
        if (!strcmp(key, GPDS_POINTINGSTICK_MIDDLE_BUTTON_TIMEOUT_KEY))
            set_middle_button_timeout(manager, xinput, client);
        else if (!strcmp(key, GPDS_POINTINGSTICK_SENSITIVITY_KEY))
            set_sensitivity(manager, xinput, client);
        else if (!strcmp(key, GPDS_POINTINGSTICK_PRESS_TO_SELECT_THRESHOLD_KEY))
            set_press_to_select_threshold(manager, xinput, client);
        break;
    default:
        break;
    }

    g_object_unref(xinput);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
