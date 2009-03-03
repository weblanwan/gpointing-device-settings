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

#ifndef __GSD_TRACK_POINT_MANAGER_H__
#define __GSD_TRACK_POINT_MANAGER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GSD_TYPE_TRACK_POINT_MANAGER            (gsd_track_point_manager_get_type ())
#define GSD_TRACK_POINT_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSD_TYPE_TRACK_POINT_MANAGER, GsdTrackPointManager))
#define GSD_TRACK_POINT_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSD_TYPE_TRACK_POINT_MANAGER, GsdTracklassPointManagerClass))
#define GSD_IS_TRACK_POINT_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSD_TYPE_TRACK_POINT_MANAGER))
#define GSD_IS_TRACK_POINT_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSD_TYPE_TRACK_POINT_MANAGER))
#define GSD_TRACK_POINT_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSD_TYPE_TRACK_POINT_MANAGER, GsdTracklassPointManagerClass))

typedef struct _GsdTrackPointManager GsdTrackPointManager;
typedef struct _GsdTrackPointManagerClass GsdTrackPointManagerClass;

struct _GsdTrackPointManager
{
    GObject parent;
};

struct _GsdTrackPointManagerClass
{
    GObjectClass parent_class;
}; 

GType gsd_track_point_manager_get_type (void) G_GNUC_CONST;

GsdTrackPointManager *gsd_track_point_manager_new   (void);
gboolean              gsd_track_point_manager_start (GsdTrackPointManager *manager,
                                                     GError              **error);
void                  gsd_track_point_manager_stop  (GsdTrackPointManager *manager);

G_END_DECLS

#endif /* __GSD_TRACK_POINT_MANAGER_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
