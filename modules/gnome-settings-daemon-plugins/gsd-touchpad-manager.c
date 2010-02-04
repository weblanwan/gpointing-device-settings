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
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>
#include <gpds-xinput-utils.h>
#include <gpds-xinput-pointer-info.h>
#include <gpds-gconf.h>

#include "gpds-touchpad-definitions.h"
#include "gpds-touchpad-xinput.h"

G_DEFINE_TYPE (GsdTouchpadManager, gsd_touchpad_manager, GSD_TYPE_POINTING_DEVICE_MANAGER)

static gboolean _start               (GsdPointingDeviceManager *manager,
                                      GError **error);
static void     _stop                (GsdPointingDeviceManager *manager);
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

    manager_class->start               = _start;
    manager_class->stop                = _stop;
    manager_class->gconf_client_notify = _gconf_client_notify;
}

DEFINE_SET_BOOLEAN_FUNCTION (palm_detection, GPDS_TOUCHPAD_PALM_DETECTION)
DEFINE_SET_BOOLEAN_FUNCTION (guest_mouse_off, GPDS_TOUCHPAD_GUEST_MOUSE_OFF)
DEFINE_SET_BOOLEAN_FUNCTION (locked_drags, GPDS_TOUCHPAD_LOCKED_DRAGS)
DEFINE_SET_BOOLEAN_FUNCTION (tap_fast_tap, GPDS_TOUCHPAD_TAP_FAST_TAP)
DEFINE_SET_BOOLEAN_FUNCTION (circular_scrolling, GPDS_TOUCHPAD_CIRCULAR_SCROLLING)
DEFINE_SET_INT_FUNCTION (touchpad_off, GPDS_TOUCHPAD_OFF)
DEFINE_SET_INT_FUNCTION (locked_drags_timeout, GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT)
DEFINE_SET_INT_FUNCTION (tap_move, GPDS_TOUCHPAD_TAP_MOVE)
DEFINE_SET_INT_FUNCTION (circular_scrolling_trigger, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER)
DEFINE_SET_BOOLEAN_PAIR_FUNCTION (two_finger_scrolling,
                                  GPDS_TOUCHPAD_TWO_FINGER_SCROLLING,
                                  GPDS_TOUCHPAD_TWO_FINGER_VERTICAL_SCROLLING_KEY,
                                  GPDS_TOUCHPAD_TWO_FINGER_HORIZONTAL_SCROLLING_KEY);
DEFINE_SET_INT_PAIR_FUNCTION (scrolling_distance,
                              GPDS_TOUCHPAD_SCROLLING_DISTANCE,
                              GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY,
                              GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY);
DEFINE_SET_INT_PAIR_FUNCTION (palm_dimensions,
                              GPDS_TOUCHPAD_PALM_DIMENSIONS,
                              GPDS_TOUCHPAD_PALM_DETECTION_WIDTH_KEY,
                              GPDS_TOUCHPAD_PALM_DETECTION_DEPTH_KEY)

static void
set_tap_time (GsdPointingDeviceManager *manager,
              GpdsXInput *xinput,
              GConfClient *gconf)
{
    gboolean disable_tapping = FALSE;
    gint tap_time;
    gint properties[1];

    if (!gsd_pointing_device_manager_get_gconf_int(manager,
                                                   gconf,
                                                   GPDS_TOUCHPAD_TAP_TIME_KEY,
                                                   &tap_time)) {
        return;
    }

    gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                  gconf,
                                                  GPDS_TOUCHPAD_DISABLE_TAPPING_KEY,
                                                  &disable_tapping);
    /*
     * If disable tapping is TRUE, do not change tapping time here
     * since tap_time is already zero (i.e. disable tapping).
     */
    if (disable_tapping)
        return;

    properties[0] = tap_time;
    gpds_xinput_set_int_properties(xinput,
                                   GPDS_TOUCHPAD_TAP_TIME,
                                   NULL,
                                   properties,
                                   1);
}

static void
set_disable_tapping (GsdPointingDeviceManager *manager,
                     GpdsXInput *xinput,
                     GConfClient *gconf)
{
    gboolean disable_tapping;
    gint tap_time = 50;
    gint properties[1];

    if (!gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                       gconf,
                                                       GPDS_TOUCHPAD_DISABLE_TAPPING_KEY,
                                                       &disable_tapping)) {
        return;
    }

    gsd_pointing_device_manager_get_gconf_int(manager,
                                              gconf,
                                              GPDS_TOUCHPAD_DISABLE_TAPPING_KEY,
                                              &tap_time);
    properties[0] = disable_tapping ? 0 : tap_time;
    gpds_xinput_set_int_properties(xinput,
                                   GPDS_TOUCHPAD_TAP_TIME,
                                   NULL,
                                   properties,
                                   1);
}

static void
set_edge_scrolling (GsdPointingDeviceManager *manager,
                    GpdsXInput *xinput,
                    GConfClient *gconf)
{
    gboolean h_enable, v_enable, c_enable = FALSE;
    gint properties[3];

    if (!gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                       gconf,
                                                       GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY,
                                                       &v_enable)) {
        return;
    }
    if (!gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                       gconf,
                                                       GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY,
                                                       &h_enable)) {
        return;
    }

    gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                  gconf,
                                                  GPDS_TOUCHPAD_CONTINUOUS_EDGE_SCROLLING_KEY,
                                                  &c_enable);

    properties[0] = v_enable ? 1 : 0;
    properties[1] = h_enable ? 1 : 0;
    properties[2] = c_enable ? 1 : 0;

    gpds_xinput_set_int_properties(xinput,
                                   GPDS_TOUCHPAD_EDGE_SCROLLING,
                                   NULL,
                                   properties,
                                   3);
}

static void
set_click_action (GsdPointingDeviceManager *manager,
                  GpdsXInput *xinput,
                  GConfClient *gconf)
{
    gint properties[3];

    if (!gsd_pointing_device_manager_get_gconf_int(manager,
                                                   gconf,
                                                   GPDS_TOUCHPAD_CLICK_ACTION_FINGER1_KEY,
                                                   &properties[0])) {
        return;
    }

    if (!gsd_pointing_device_manager_get_gconf_int(manager,
                                                   gconf,
                                                   GPDS_TOUCHPAD_CLICK_ACTION_FINGER2_KEY,
                                                   &properties[1])) {
        return;
    }

    if (!gsd_pointing_device_manager_get_gconf_int(manager,
                                                   gconf,
                                                   GPDS_TOUCHPAD_CLICK_ACTION_FINGER3_KEY,
                                                   &properties[2])) {
        return;
    }

    gpds_xinput_set_int_properties(xinput,
                                   GPDS_TOUCHPAD_CLICK_ACTION,
                                   NULL,
                                   properties,
                                   3);
}

static void
set_move_speed (GsdPointingDeviceManager *manager,
                GpdsXInput *xinput,
                GConfClient *gconf)
{
    gdouble properties[4];

    if (!gsd_pointing_device_manager_get_gconf_float(manager,
                                                     gconf,
                                                     GPDS_TOUCHPAD_MINIMUM_SPEED_KEY,
                                                     &properties[0])) {
        return;
    }

    if (!gsd_pointing_device_manager_get_gconf_float(manager,
                                                     gconf,
                                                     GPDS_TOUCHPAD_MAXIMUM_SPEED_KEY,
                                                     &properties[1])) {
        return;
    }

    if (!gsd_pointing_device_manager_get_gconf_float(manager,
                                                     gconf,
                                                     GPDS_TOUCHPAD_ACCELERATION_FACTOR_KEY,
                                                     &properties[2])) {
        return;
    }

    if (!gsd_pointing_device_manager_get_gconf_float(manager,
                                                     gconf,
                                                     GPDS_TOUCHPAD_TRACKSTICK_SPEED_KEY,
                                                     &properties[3])) {
        return;
    }

    gpds_xinput_set_float_properties(xinput,
                                     GPDS_TOUCHPAD_MOVE_SPEED,
                                     NULL,
                                     properties,
                                     4);
}

static void
set_disable_while_other_device_exists (GsdPointingDeviceManager *manager,
                                       GpdsXInput *xinput,
                                       GConfClient *gconf)
{
    gint properties[1];
    gboolean disable = FALSE;
    GList *node, *pointer_infos;;
    gboolean exists_other_device = FALSE;
    gint use_type;
    const gchar *device_name;

    if (!gsd_pointing_device_manager_get_gconf_boolean(manager,
                                                       gconf,
                                                       GPDS_TOUCHPAD_DISABLE_WHILE_OTHER_DEVICE_EXISTS_KEY,
                                                       &disable)) {
        return;
    }

    device_name = gpds_xinput_get_device_name(xinput);
    pointer_infos = gpds_xinput_utils_collect_pointer_infos();
    for (node = pointer_infos; node; node = g_list_next(node)) {
        GpdsXInputPointerInfo *info = node->data;
        const gchar *name = gpds_xinput_pointer_info_get_name(info);
        if (g_ascii_strcasecmp(device_name, name) &&
            strcmp(name, "Macintosh mouse button emulation")) {
            exists_other_device = TRUE;
            break;
        }
    }
    g_list_foreach(pointer_infos, (GFunc)gpds_xinput_pointer_info_free, NULL);
    g_list_free(pointer_infos);

    gsd_pointing_device_manager_get_gconf_int(manager,
                                              gconf,
                                              GPDS_TOUCHPAD_OFF_KEY,
                                              &use_type);
    if (disable && exists_other_device)
        properties[0] = GPDS_TOUCHPAD_USE_TYPE_OFF;
    else
        properties[0] = use_type;

    gpds_xinput_set_int_properties(xinput,
                                   GPDS_TOUCHPAD_OFF,
                                   NULL,
                                   properties,
                                   1);
}

static GdkFilterReturn
device_presence_filter (GdkXEvent *xevent,
                        GdkEvent  *event,
                        gpointer   data)
{
    XEvent *xev = (XEvent *)xevent;
    XEventClass class_presence;
    int xi_presence;
    GpdsXInput *xinput;
    GsdPointingDeviceManager *manager = GSD_POINTING_DEVICE_MANAGER(data);

    xinput = gsd_pointing_device_manager_get_xinput(manager);
    if (!xinput)
        return GDK_FILTER_CONTINUE;

    DevicePresence(gdk_x11_get_default_xdisplay(), xi_presence, class_presence);

    if (xev->type == xi_presence) {
        XDevicePresenceNotifyEvent *notify_event = (XDevicePresenceNotifyEvent *)xev;
        if (notify_event->devchange == DeviceEnabled ||
            notify_event->devchange == DeviceRemoved) {
            set_disable_while_other_device_exists(manager,
                                                  xinput,
                                                  gconf_client_get_default());
        }
    }
    g_object_unref(xinput);

    return GDK_FILTER_CONTINUE;
}


static void
add_device_presence_filter (GsdPointingDeviceManager *manager)
{
    Display *display;
    XEventClass class_presence;
    gint xi_presence;

    gint op_code, event, error;

    if (!XQueryExtension(GDK_DISPLAY(),
                         "XInputExtension",
                         &op_code,
                         &event,
                         &error)) {
        return;
    }

    display = gdk_x11_get_default_xdisplay();

    gdk_error_trap_push();
    DevicePresence(display, xi_presence, class_presence);
    XSelectExtensionEvent(display,
                          RootWindow(display, DefaultScreen(display)),
                          &class_presence, 1);
    gdk_flush();
    if (!gdk_error_trap_pop())
        gdk_window_add_filter(NULL, device_presence_filter, manager);
}

static void
remove_device_presence_filter (GsdPointingDeviceManager *manager)
{
    gdk_window_remove_filter(NULL, device_presence_filter, manager);
}

static gboolean
start_manager (GsdPointingDeviceManager *manager)
{
    GpdsXInput *xinput;
    GConfClient *gconf;

    xinput = gsd_pointing_device_manager_get_xinput(manager);
    if (!xinput)
        return FALSE;

    gpds_touchpad_xinput_setup_property_entries(xinput);

    gconf = gconf_client_get_default();
    if (!gconf) {
        g_object_unref(xinput);
        return FALSE;
    }

    set_touchpad_off(manager, xinput, gconf);
    set_guest_mouse_off(manager, xinput, gconf);
    set_palm_detection(manager, xinput, gconf);
    set_locked_drags(manager, xinput, gconf);
    set_locked_drags_timeout(manager, xinput, gconf);
    set_tap_fast_tap(manager, xinput, gconf);
    set_disable_tapping(manager, xinput, gconf);
    set_tap_time(manager, xinput, gconf);
    set_tap_move(manager, xinput, gconf);
    set_edge_scrolling(manager, xinput, gconf);
    set_scrolling_distance(manager, xinput, gconf);
    set_circular_scrolling(manager, xinput, gconf);
    set_circular_scrolling_trigger(manager, xinput, gconf);
    set_two_finger_scrolling(manager, xinput, gconf);
    set_click_action(manager, xinput, gconf);
    set_move_speed(manager, xinput, gconf);

    set_disable_while_other_device_exists(manager, xinput, gconf);
    add_device_presence_filter(manager);

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
    remove_device_presence_filter(manager);
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

    gpds_touchpad_xinput_setup_property_entries(xinput);

    value = gconf_entry_get_value(entry);
    key = gpds_gconf_get_key_from_path(gconf_entry_get_key(entry));

    switch (value->type) {
    case GCONF_VALUE_BOOL:
        if (!strcmp(key, GPDS_TOUCHPAD_DISABLE_WHILE_OTHER_DEVICE_EXISTS_KEY)) {
            set_disable_while_other_device_exists(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_PALM_DETECTION_KEY)) {
            set_palm_detection(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_GUEST_MOUSE_OFF_KEY)) {
            set_guest_mouse_off(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_LOCKED_DRAGS_KEY)) {
            set_locked_drags(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_TAP_FAST_TAP_KEY)) {
            set_tap_fast_tap(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY)) {
            set_circular_scrolling(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_CONTINUOUS_EDGE_SCROLLING_KEY)) {
            set_edge_scrolling(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_TWO_FINGER_VERTICAL_SCROLLING_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_TWO_FINGER_HORIZONTAL_SCROLLING_KEY)) {
            set_two_finger_scrolling(manager, xinput, client);
        }
        break;
    case GCONF_VALUE_INT:
        if (!strcmp(key, GPDS_TOUCHPAD_OFF_KEY)) {
            set_touchpad_off(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT_KEY)) {
            set_locked_drags_timeout(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_TAP_TIME_KEY)) {
            set_tap_time(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_TAP_MOVE_KEY)) {
            set_tap_move(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY)) {
            set_scrolling_distance(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_PALM_DETECTION_WIDTH_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_PALM_DETECTION_DEPTH_KEY)) {
            set_palm_dimensions(manager, xinput, client);
        } else if (!strcmp(key, GPDS_TOUCHPAD_CLICK_ACTION_FINGER1_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_CLICK_ACTION_FINGER2_KEY) ||
                   !strcmp(key, GPDS_TOUCHPAD_CLICK_ACTION_FINGER3_KEY)) {
            set_click_action(manager, xinput, client);
        }
        break;
    case GCONF_VALUE_FLOAT:
        if (!strcmp(key, GPDS_TOUCHPAD_MINIMUM_SPEED_KEY) ||
            !strcmp(key, GPDS_TOUCHPAD_MAXIMUM_SPEED_KEY) ||
            !strcmp(key, GPDS_TOUCHPAD_ACCELERATION_FACTOR_KEY) ||
            !strcmp(key, GPDS_TOUCHPAD_TRACKSTICK_SPEED_KEY)) {
            set_move_speed(manager, xinput, client);
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
