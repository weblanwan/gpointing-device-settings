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

#ifndef __GPDS_MOUSE_XINPUT_H__
#define __GPDS_MOUSE_XINPUT_H__

#include <glib.h>

typedef enum {
    GPDS_MOUSE_MIDDLE_BUTTON_EMULATION,
    GPDS_MOUSE_MIDDLE_BUTTON_TIMEOUT,
    GPDS_MOUSE_WHEEL_EMULATION,
    GPDS_MOUSE_WHEEL_EMULATION_INERTIA,
    GPDS_MOUSE_WHEEL_EMULATION_AXES,
    GPDS_MOUSE_WHEEL_EMULATION_TIMEOUT,
    GPDS_MOUSE_WHEEL_EMULATION_BUTTON,
    GPDS_MOUSE_DRAG_LOCK_BUTTONS,
} GpdsMouseProperty;

typedef struct _GpdsMouseXInputProperty GpdsMouseXInputProperty;
struct _GpdsMouseXInputProperty
{
    GpdsMouseProperty property;
    const gchar *name;
    gint format_type;
    gint max_value_count;
};

const gchar *gpds_mouse_xinput_get_name            (GpdsMouseProperty property);
gint         gpds_mouse_xinput_get_format_type     (GpdsMouseProperty property);
gint         gpds_mouse_xinput_get_max_value_count (GpdsMouseProperty property);
const gchar *gpds_mouse_xinput_find_device_name    (void);


#endif /* __GPDS_MOUSE_XINPUT_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
