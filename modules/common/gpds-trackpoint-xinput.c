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

#include "gpds-trackpoint-xinput.h"
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>

static GpdsTrackPointXInputProperty properties[] = {
    {GPDS_TRACK_POINT_MIDDLE_BUTTON_EMULATION, "Evdev Middle Button Emulation", 8, 1},
    {GPDS_TRACK_POINT_MIDDLE_BUTTON_TIMEOUT, "Evdev Middle Button Timeout", 32, 1},
    {GPDS_TRACK_POINT_WHEEL_EMULATION, "Evdev Wheel Emulation", 8, 1},
    {GPDS_TRACK_POINT_WHEEL_EMULATION_INERTIA, "Evdev Wheel Emulation Inertia", 16, 1},
    {GPDS_TRACK_POINT_WHEEL_EMULATION_AXES, "Evdev Wheel Emulation Axes", 8, 4},
    {GPDS_TRACK_POINT_WHEEL_EMULATION_TIMEOUT, "Evdev Wheel Emulation Timeout", 16, 1},
    {GPDS_TRACK_POINT_WHEEL_EMULATION_BUTTON, "Evdev Wheel Emulation Button", 8, 1},
    {GPDS_TRACK_POINT_DRAG_LOCK_BUTTONS, "Evdev Drag Lock Buttons", 8, 2}
};

static const gint n_properties = G_N_ELEMENTS(properties);

const gchar *
gpds_track_point_xinput_get_name (GpdsTrackPointProperty property)
{
    gint i;

    for (i = 0; i < n_properties; i++) {
        if (property == properties[i].property)
            return properties[i].name;
    }

    return NULL;
}

gint
gpds_track_point_xinput_get_format_type (GpdsTrackPointProperty property)
{
    gint i;

    for (i = 0; i < n_properties; i++) {
        if (property == properties[i].property)
            return properties[i].format_type;
    }

    return -1;
}

gint
gpds_track_point_xinput_get_max_value_count (GpdsTrackPointProperty property)
{
    gint i;

    for (i = 0; i < n_properties; i++) {
        if (property == properties[i].property)
            return properties[i].max_value_count;
    }

    return -1;
}


/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
