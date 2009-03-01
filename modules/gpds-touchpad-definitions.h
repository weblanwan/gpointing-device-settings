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

#ifndef __GPDS_TOUCHPAD_GCONF_H__
#define __GPDS_TOUCHPAD_GCONF_H__

#define GPDS_TOUCHPAD_DEVICE_NAME "SynPS/2 Synaptics TouchPad"

#define GPDS_TOUCHPAD_EDGES                       "Synaptics Edges"
#define GPDS_TOUCHPAD_FINGER                      "Synaptics Finger"
#define GPDS_TOUCHPAD_TAP_TIME                    "Synaptics Tap Time"
#define GPDS_TOUCHPAD_TAP_MOVE                    "Synaptics Tap Move"
#define GPDS_TOUCHPAD_TAP_DURATIONS               "Synaptics Tap Durations"
#define GPDS_TOUCHPAD_TAP_FAST_TAP                "Synaptics Tap FastTap"
#define GPDS_TOUCHPAD_MIDDLE_BUTTON_TIMEOUT       "Synaptics Middle Button Timeout"
#define GPDS_TOUCHPAD_TWO_FINGER_PRESSURE         "Synaptics Two-Finger Pressure"
#define GPDS_TOUCHPAD_SCROLLING_DISTANCE          "Synaptics Scrolling Distance"
#define GPDS_TOUCHPAD_EDGE_SCROLLING              "Synaptics Edge Scrolling"
#define GPDS_TOUCHPAD_TWO_FINGER_SCROLLING        "Synaptics Two-Finger Scrolling"
#define GPDS_TOUCHPAD_EDGE_MOTION_PRESSURE        "Synaptics Edge Motion Pressure"
#define GPDS_TOUCHPAD_EDGE_MOTION_SPEED           "Synaptics Edge Motion Speed"
#define GPDS_TOUCHPAD_EDGE_MOTION_ALWAYS          "Synaptics Edge Motion Always"
#define GPDS_TOUCHPAD_BUTTON_SCROLLING            "Synaptics Button Scrolling"
#define GPDS_TOUCHPAD_BUTTON_SCROLLING_REPEAT     "Synaptics Button Scrolling Repeat"
#define GPDS_TOUCHPAD_SCROLLING_TIME              "Synaptics Button Scrolling Time"
#define GPDS_TOUCHPAD_OFF                         "Synaptics Off"
#define GPDS_TOUCHPAD_GUESTMOUSE_OFF              "Synaptics Guestmouse Off"
#define GPDS_TOUCHPAD_LOCKED_DRAGS                "Synaptics Locked Drags"
#define GPDS_TOUCHPAD_LOCKED_DRAGS_TIMEOUT        "Synaptics Locked Drags Timeout"
#define GPDS_TOUCHPAD_TAP_ACTION                  "Synaptics Tap Action"
#define GPDS_TOUCHPAD_CLICK_ACTION                "Synaptics Click Action"
#define GPDS_TOUCHPAD_CIRCULAR_SCROLLING          "Synaptics Circular Scrolling"
#define GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER  "Synaptics Circular Scrolling Trigger"
#define GPDS_TOUCHPAD_CIRCULAR_PAD                "Synaptics Circular Pad"
#define GPDS_TOUCHPAD_PALM_DETECTION              "Synaptics Palm Detection"
#define GPDS_TOUCHPAD_PALM_DIMENSIONS             "Synaptics Palm Dimensions"
#define GPDS_TOUCHPAD_PRESSURE_MOTION             "Synaptics Pressure Motion"
#define GPDS_TOUCHPAD_GRAB_EVENT_DEVICE           "Synaptics Grab Event Device"

#define GPDS_TOUCHPAD_GCONF_DIR                   "/desktop/gnome/peripherals/touchpad"

#define GPDS_TOUCHPAD_EDGES_KEY                         GPDS_TOUCHPAD_GCONF_DIR "/edges"
#define GPDS_TOUCHPAD_OFF_KEY                           GPDS_TOUCHPAD_GCONF_DIR "/off"
#define GPDS_TOUCHPAD_TAP_TIME_KEY                      GPDS_TOUCHPAD_GCONF_DIR "/tap_time"
#define GPDS_TOUCHPAD_TAP_FAST_TAP_KEY                  GPDS_TOUCHPAD_GCONF_DIR "/tap_fast_tap"
#define GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_DISTANCE_KEY GPDS_TOUCHPAD_GCONF_DIR "/horizontal_scrolling_distance"
#define GPDS_TOUCHPAD_HORIZONTAL_SCROLLING_KEY          GPDS_TOUCHPAD_GCONF_DIR "/horizontal_scrolling"
#define GPDS_TOUCHPAD_VERTICAL_SCROLLING_DISTANCE_KEY   GPDS_TOUCHPAD_GCONF_DIR "/vertical_scrolling_distance"
#define GPDS_TOUCHPAD_VERTICAL_SCROLLING_KEY            GPDS_TOUCHPAD_GCONF_DIR "/vertical_scrolling"
#define GPDS_TOUCHPAD_CIRCULAR_SCROLLING_KEY            GPDS_TOUCHPAD_GCONF_DIR "/circular_scrolling"
#define GPDS_TOUCHPAD_CIRCULAR_SCROLLING_DISTANCE_KEY   GPDS_TOUCHPAD_GCONF_DIR "/circular_scrolling_distance"
#define GPDS_TOUCHPAD_CIRCULAR_SCROLLING_TRIGGER_KEY    GPDS_TOUCHPAD_GCONF_DIR "/circular_scrolling_trigger"

#endif /* __GPDS_TOUCHPAD_GCONF_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
