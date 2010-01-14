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

#ifndef __GPDS_TOUCHPAD_XINPUT_H__
#define __GPDS_TOUCHPAD_XINPUT_H__

#include <glib-object.h>
#include <gpds-xinput.h>

typedef enum {
    GPDS_TOUCHPAD_EDGES,
    GPDS_TOUCHPAD_FINGER,
    GPDS_TOUCHPAD_TAP_TIME,
    GPDS_TOUCHPAD_TAP_MOVE,
    GPDS_TOUCHPAD_TAP_DURATIONS,
    GPDS_TOUCHPAD_TAP_FAST_TAP,
    GPDS_TOUCHPAD_MIDDLE_BUTTON_TIMEOUT,
    GPDS_TOUCHPAD_TWO_FINGER_PRESSURE,
    GPDS_TOUCHPAD_SCROLLING_DISTANCE,
    GPDS_TOUCHPAD_EDGE_SCROLLING,
    GPDS_TOUCHPAD_TWO_FINGER_SCROLLING,
    GPDS_TOUCHPAD_MOVE_SPEED,
    GPDS_TOUCHPAD_EDGE_MOTION_PRESSURE,
    GPDS_TOUCHPAD_EDGE_MOTION_SPEED,
    GPDS_TOUCHPAD_BUTTON_SCROLLING,
    GPDS_TOUCHPAD_BUTTON_SCROLLING_REPEAT,
    GPDS_TOUCHPAD_SCROLLING_TIME,
    GPDS_TOUCHPAD_OFF,
    GPDS_TOUCHPAD_GUEST_MOUSE_OFF,
    GPDS_TOUCHPAD_LOCKED_DRAGS,
    GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT,
    GPDS_TOUCHPAD_TAP_ACTION,
    GPDS_TOUCHPAD_CLICK_ACTION,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_DISTANCE,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER,
    GPDS_TOUCHPAD_CIRCULAR_PAD,
    GPDS_TOUCHPAD_PALM_DETECTION,
    GPDS_TOUCHPAD_PALM_DIMENSIONS,
    GPDS_TOUCHPAD_PRESSURE_MOTION,
    GPDS_TOUCHPAD_GRAB_EVENT_DEVICE
} GpdsTouchpadProperty;

typedef enum {
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_ANY,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_TOP,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_TOP_RIGHT,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_RIGHT,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_RIGHT_BOTTOM,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_BOTTOM,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_BOTTOM_LEFT,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_LEFT,
    GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_LEFT_TOP
} GpdsTouchpadCircularScrollingTrigger;

typedef enum {
    GPDS_TOUCHPAD_USE_TYPE_NORMAL,
    GPDS_TOUCHPAD_USE_TYPE_OFF,
    GPDS_TOUCHPAD_USE_TYPE_TAPPING_AND_SCROLLING_OFF,
} GpdsTouchpadUseType;

GpdsXInput *gpds_touchpad_xinput_new                    (const gchar *device_name);
void        gpds_touchpad_xinput_setup_property_entries (GpdsXInput *xinput);

#endif /* __GPDS_TOUCHPAD_XINPUT_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
