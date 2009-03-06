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

#include "gpds-touchpad-xinput.h"
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>
#include <gpds-xinput-utils.h>

static const gchar *touchpad_device_names[] = {
    "SynPS/2 Synaptics TouchPad",
    "AlpsPS/2 ALPS GlidePoint"
};

static const gint n_touchpad_device_names = G_N_ELEMENTS(touchpad_device_names);

static GpdsTouchpadXInputProperty properties[] = {
    {GPDS_TOUCHPAD_EDGES,                      "Synaptics Edges", G_TYPE_INT, 32, 4},
    {GPDS_TOUCHPAD_FINGER,                     "Synaptics Finger", G_TYPE_INT, 32, 3},
    {GPDS_TOUCHPAD_TAP_TIME,                   "Synaptics Tap Time", G_TYPE_INT, 32, 1},
    {GPDS_TOUCHPAD_TAP_MOVE,                   "Synaptics Tap Move", G_TYPE_INT, 32, 1},
    {GPDS_TOUCHPAD_TAP_DURATIONS,              "Synaptics Tap Durations", G_TYPE_INT, 32, 3},
    {GPDS_TOUCHPAD_TAP_FAST_TAP,               "Synaptics Tap FastTap", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_MIDDLE_BUTTON_TIMEOUT,      "Synaptics Middle Button Timeout", G_TYPE_INT, 32, 1},
    {GPDS_TOUCHPAD_TWO_FINGER_PRESSURE,        "Synaptics Two-Finger Pressure", G_TYPE_INT, 32, 1},
    {GPDS_TOUCHPAD_SCROLLING_DISTANCE,         "Synaptics Scrolling Distance", G_TYPE_INT, 32, 2},
    {GPDS_TOUCHPAD_EDGE_SCROLLING,             "Synaptics Edge Scrolling", G_TYPE_INT, 8, 3},
    {GPDS_TOUCHPAD_TWO_FINGER_SCROLLING,       "Synaptics Two-Finger Scrolling", G_TYPE_INT, 8, 2},
    {GPDS_TOUCHPAD_SYNAPTICS_PROP_SPEED,       "Synaptics Move Speed", G_TYPE_FLOAT, 0, 4},
    {GPDS_TOUCHPAD_EDGE_MOTION_PRESSURE,       "Synaptics Edge Motion Pressure", G_TYPE_INT, 32, 2},
    {GPDS_TOUCHPAD_EDGE_MOTION_SPEED,          "Synaptics Edge Motion Speed", G_TYPE_INT, 32, 2},
    {GPDS_TOUCHPAD_BUTTON_SCROLLING,           "Synaptics Button Scrolling", G_TYPE_INT, 8, 2},
    {GPDS_TOUCHPAD_BUTTON_SCROLLING_REPEAT,    "Synaptics Button Scrolling Repeat", G_TYPE_INT, 8, 2},
    {GPDS_TOUCHPAD_SCROLLING_TIME,             "Synaptics Button Scrolling Time", G_TYPE_INT, 32, 1},
    {GPDS_TOUCHPAD_OFF,                        "Synaptics Off", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_GUESTMOUSE_OFF,             "Synaptics Guestmouse Off", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_LOCKED_DRAGS,               "Synaptics Locked Drags", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT,       "Synaptics Locked Drags Timeout", G_TYPE_INT, 32, 1},
    {GPDS_TOUCHPAD_TAP_ACTION,                 "Synaptics Tap Action", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_CLICK_ACTION,               "Synaptics Click Action", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_CIRCULAR_SCROLLING,         "Synaptics Circular Scrolling", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_CIRCULAR_SCROLLING_DISTANCE,"Synaptics Circular Scrolling Distance", G_TYPE_FLOAT, 0, 1},
    {GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER, "Synaptics Circular Scrolling Trigger", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_CIRCULAR_PAD,               "Synaptics Circular Pad", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_PALM_DETECTION,             "Synaptics Palm Detection", G_TYPE_INT, 8, 1},
    {GPDS_TOUCHPAD_PALM_DIMENSIONS,            "Synaptics Palm Dimensions", G_TYPE_INT, 32, 2},
    {GPDS_TOUCHPAD_PRESSURE_MOTION,            "Synaptics Pressure Motion", G_TYPE_INT, 32, 2},
    {GPDS_TOUCHPAD_GRAB_EVENT_DEVICE,          "Synaptics Grab Event Device", G_TYPE_INT, 8, 1},
};

static const gint n_properties = G_N_ELEMENTS(properties);

const gchar *
gpds_touchpad_xinput_get_name (GpdsTouchpadProperty property)
{
    gint i;

    for (i = 0; i < n_properties; i++) {
        if (property == properties[i].property)
            return properties[i].name;
    }

    return NULL;
}

gint
gpds_touchpad_xinput_get_format_type (GpdsTouchpadProperty property)
{
    gint i;

    for (i = 0; i < n_properties; i++) {
        if (property == properties[i].property)
            return properties[i].format_type;
    }

    return -1;
}

gint
gpds_touchpad_xinput_get_max_value_count (GpdsTouchpadProperty property)
{
    gint i;

    for (i = 0; i < n_properties; i++) {
        if (property == properties[i].property)
            return properties[i].max_value_count;
    }

    return -1;
}

const gchar *
gpds_touchpad_xinput_find_device_name (void)
{
    gint i;

    for (i = 0; i < n_touchpad_device_names; i++) {
        if (gpds_xinput_utils_exist_device(touchpad_device_names[i]))
            return touchpad_device_names[i];
    }
    return NULL;
}


/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
