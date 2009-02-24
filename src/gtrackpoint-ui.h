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

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define G_TYPE_TRACK_POINT_UI            (g_track_point_ui_get_type ())
#define G_TRACK_POINT_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_TRACK_POINT_UI, GTrackPointUI))
#define G_TRACK_POINT_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_TRACK_POINT_UI, GTrackPointUIClass))
#define G_IS_TRACK_POINT_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_TRACK_POINT_UI))
#define G_IS_TRACK_POINT_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_TRACK_POINT_UI))
#define G_TRACK_POINT_UI_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), G_TYPE_TRACK_POINT_UI, GTrackPointUIClass))

typedef struct _GTrackPointUI GTrackPointUI;
typedef struct _GTrackPointUIClass GTrackPointUIClass;

struct _GTrackPointUI
{
    GObject parent;
};

struct _GTrackPointUIClass
{
    GObjectClass parent_class;
};

void g_track_point_create_window (void);

GType          g_track_point_ui_get_type   (void) G_GNUC_CONST;
GTrackPointUI *g_track_point_ui_new        (void);
GtkWidget     *g_track_point_ui_get_widget (GTrackPointUI *ui);
GtkWidget     *g_track_point_ui_get_label_widget
                                           (GTrackPointUI *ui);

G_END_DECLS

#endif /* __GTRACKPOINT_UI_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
