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

#ifndef __GTRACKPOINT_UI_H__
#define __GTRACKPOINT_UI_H__

#include "gpds-ui.h"

G_BEGIN_DECLS

#define GPDS_TYPE_TRACK_POINT_UI            (gpds_track_point_ui_get_type ())
#define GPDS_TRACK_POINT_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_TRACK_POINT_UI, GpdsTrackPointUI))
#define GPDS_TRACK_POINT_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_TRACK_POINT_UI, GpdsTrackPointUIClass))
#define G_IS_TRACK_POINT_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_TRACK_POINT_UI))
#define G_IS_TRACK_POINT_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_TRACK_POINT_UI))
#define GPDS_TRACK_POINT_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_TRACK_POINT_UI, GpdsTrackPointUIClass))

typedef struct _GpdsTrackPointUI GpdsTrackPointUI;
typedef struct _GpdsTrackPointUIClass GpdsTrackPointUIClass;

struct _GpdsTrackPointUI
{
    GpdsUI parent;
};

struct _GpdsTrackPointUIClass
{
    GpdsUIClass parent_class;
};

GType          gpds_track_point_ui_get_type   (void) G_GNUC_CONST;
GpdsTrackPointUI *gpds_track_point_ui_new        (void);
GtkWidget     *gpds_track_point_ui_get_widget (GpdsTrackPointUI *ui);
GtkWidget     *gpds_track_point_ui_get_label_widget
                                           (GpdsTrackPointUI *ui);

G_END_DECLS

#endif /* __GTRACKPOINT_UI_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
