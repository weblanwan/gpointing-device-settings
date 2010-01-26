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

#include "gpds-pointingstick-xinput.h"
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gpds-xinput.h>
#include <gpds-xinput-utils.h>

static const GpdsXInputPropertyEntry entries[] = {
    {GPDS_POINTINGSTICK_SENSITIVITY,               "PointingStick Sensitivity",               G_TYPE_INT,  8, 1},
    {GPDS_POINTINGSTICK_SCROLLING,                 "PointingStick Scrolling",                 G_TYPE_INT,  8, 1},
    {GPDS_POINTINGSTICK_MIDDLE_BUTTON_TIMEOUT,     "PointingStick Middle Button Timeout",     G_TYPE_INT, 16, 1},
    {GPDS_POINTINGSTICK_PRESS_TO_SELECT,           "PointingStick Press to Select",           G_TYPE_INT,  8, 1},
    {GPDS_POINTINGSTICK_PRESS_TO_SELECT_THRESHOLD, "PointingStick Press to Select Threshold", G_TYPE_INT,  8, 1}
};

static const gint n_entries = G_N_ELEMENTS(entries);

GpdsXInput *
gpds_pointingstick_xinput_new (const gchar *device_name)
{
    GpdsXInput *xinput;

    xinput = gpds_xinput_new(device_name);
    gpds_xinput_register_property_entries(xinput, entries, n_entries);

    return xinput;
}

void
gpds_pointingstick_xinput_setup_property_entries (GpdsXInput *xinput)
{
    gpds_xinput_register_property_entries(xinput, entries, n_entries);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
