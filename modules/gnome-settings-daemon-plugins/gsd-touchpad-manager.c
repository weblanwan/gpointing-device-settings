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
#include <gpds-gconf.h>

#include "gpds-touchpad-definitions.h"
#include "gpds-touchpad-xinput.h"

G_DEFINE_TYPE (GsdTouchpadManager, gsd_touchpad_manager, GSD_TYPE_POINTING_DEVICE_MANAGER)

static gboolean start                (GsdPointingDeviceManager *manager,
                                      GError **error);
static void     stop                 (GsdPointingDeviceManager *manager);
static void     _gconf_client_notify (GsdPointingDeviceManager *manager,
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

    manager_class->start               = start;
    manager_class->stop                = stop;
    manager_class->gconf_client_notify = _gconf_client_notify;
}

static GpdsXInput *
get_xinput (GsdPointingDeviceManager *manager)
{
    const gchar *device_name;

    device_name = gsd_pointing_device_manager_get_device_name(manager);
    if (!device_name)
        return NULL;

    if (!gpds_xinput_utils_exist_device(device_name))
        return NULL;

    return gpds_xinput_new(device_name);
}

#define DEFINE_SET_VALUE_FUNCTION(function_name, key_name, value_type)              \
static void                                                                         \
set_ ## function_name (GsdPointingDeviceManager *manager,                           \
                       GpdsXInput *xinput,                                          \
                       GConfClient *gconf)                                          \
{                                                                                   \
    g ## value_type value;                                                          \
    gint properties[1];                                                             \
    gchar *key;                                                                     \
    gboolean value_exist;                                                           \
    key = gsd_pointing_device_manager_build_gconf_key(manager, key_name ## _KEY);   \
    value_exist = gpds_gconf_get_ ## value_type (gconf, key_name ## _KEY, &value);  \
    g_free(key);                                                                    \
    if (!value_exist)                                                               \
        return;                                                                     \
    properties[0] = value;                                                          \
    gpds_xinput_set_int_properties(xinput,                                          \
                                   gpds_touchpad_xinput_get_name(key_name),         \
                                   gpds_touchpad_xinput_get_format_type(key_name),  \
                                   NULL,                                            \
                                   properties,                                      \
                                   1);                                              \
}

#define DEFINE_SET_BOOLEAN_FUNCTION(function_name, key_name)                    \
    DEFINE_SET_VALUE_FUNCTION(function_name, key_name, boolean)

#define DEFINE_SET_INT_FUNCTION(function_name, key_name)                        \
    DEFINE_SET_VALUE_FUNCTION(function_name, key_name, int)

DEFINE_SET_BOOLEAN_FUNCTION (tap_fast_tap, GPDS_TOUCHPAD_TAP_FAST_TAP)
DEFINE_SET_BOOLEAN_FUNCTION (circular_scrolling, GPDS_TOUCHPAD_CIRCULAR_SCROLLING)
DEFINE_SET_INT_FUNCTION (touchpad_off, GPDS_TOUCHPAD_OFF)
DEFINE_SET_INT_FUNCTION (tap_time, GPDS_TOUCHPAD_TAP_TIME)

static void
set_horizontal_and_vertical_scrolling (GsdPointingDeviceManager *manager,
                                       GpdsXInput *xinput,
                                       GConfClient *gconf)
{
    gboolean h_enable, v_enable;
    gint properties[3];

    if (!gpds_gconf_get_boolean(gconf, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY, &v_enable))
        return;
    if (!gpds_gconf_get_boolean(gconf, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY, &h_enable))
        return;

    properties[0] = v_enable ? 1 : 0;
    properties[1] = h_enable ? 1 : 0;
    properties[2] = 0;

    gpds_xinput_set_int_properties(xinput,
                                   gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_EDGE_SCROLLING),
                                   gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_EDGE_SCROLLING),
                                   NULL,
                                   properties,
                                   3);
}

static void
set_horizontal_and_vertical_scrolling_distance (GsdPointingDeviceManager *manager,
                                                GpdsXInput *xinput,
                                                GConfClient *gconf)
{
    gint h_distance, v_distance;
    gint properties[2];

    if (!gpds_gconf_get_boolean(gconf, GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY, &v_distance))
        return;
    if (!gpds_gconf_get_boolean(gconf, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY, &h_distance))
        return;

    properties[0] = v_distance ? 1 : 0;
    properties[1] = h_distance ? 1 : 0;

    gpds_xinput_set_int_properties(xinput,
                                   gpds_touchpad_xinput_get_name(GPDS_TOUCHPAD_SCROLLING_DISTANCE),
                                   gpds_touchpad_xinput_get_format_type(GPDS_TOUCHPAD_SCROLLING_DISTANCE),
                                   NULL,
                                   properties,
                                   2);
}

static gboolean
start (GsdPointingDeviceManager *manager, GError **error)
{
    GpdsXInput *xinput;
    GConfClient *gconf;

    xinput = get_xinput(manager);
    if (!xinput)
        return FALSE;

    gconf = gconf_client_get_default();
    if (!gconf) {
        g_object_unref(xinput);
        return FALSE;
    }

    set_tap_fast_tap(manager, xinput, gconf);
    set_circular_scrolling(manager, xinput, gconf);
    set_touchpad_off(manager, xinput, gconf);
    set_tap_time(manager, xinput, gconf);
    set_horizontal_and_vertical_scrolling(manager, xinput, gconf);

    g_object_unref(gconf);
    g_object_unref(xinput);

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

    value = gconf_entry_get_value(entry);
    key = gpds_gconf_get_key_from_path(gconf_entry_get_key(entry));

    switch (value->type) {
    case GCONF_VALUE_BOOL:
        if (!strcmp(key, GPDS_TOUCHPAD_TAP_FAST_TAP_KEY)) {
            set_tap_fast_tap(manager, xinput, client);
        } else  if (!strcmp(key, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY)) {
            set_circular_scrolling(manager, xinput, client);
        } else  if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY) ||
                    !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY)) {
            set_horizontal_and_vertical_scrolling(manager, xinput, client);
        }
        break;
    case GCONF_VALUE_INT:
        if (!strcmp(key, GPDS_TOUCHPAD_OFF_KEY)) {
            set_touchpad_off(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_TAP_TIME_KEY)) {
            set_tap_time(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY)) {
            set_horizontal_and_vertical_scrolling_distance(manager, xinput, client);
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
