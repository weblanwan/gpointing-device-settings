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

#ifndef __GSD_POINTINGSTICK_MANAGER_H__
#define __GSD_POINTINGSTICK_MANAGER_H__

#include "gsd-pointing-device-manager.h"

G_BEGIN_DECLS

#define GSD_TYPE_POINTINGSTICK_MANAGER            (gsd_pointingstick_manager_get_type ())
#define GSD_POINTINGSTICK_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSD_TYPE_POINTINGSTICK_MANAGER, GsdPointingStickManager))
#define GSD_POINTINGSTICK_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSD_TYPE_POINTINGSTICK_MANAGER, GsdTracklassPointManagerClass))
#define GSD_IS_POINTINGSTICK_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSD_TYPE_POINTINGSTICK_MANAGER))
#define GSD_IS_POINTINGSTICK_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSD_TYPE_POINTINGSTICK_MANAGER))
#define GSD_POINTINGSTICK_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSD_TYPE_POINTINGSTICK_MANAGER, GsdTracklassPointManagerClass))

typedef struct _GsdPointingStickManager GsdPointingStickManager;
typedef struct _GsdPointingStickManagerClass GsdPointingStickManagerClass;

struct _GsdPointingStickManager
{
    GsdPointingDeviceManager parent;
};

struct _GsdPointingStickManagerClass
{
    GsdPointingDeviceManagerClass parent_class;
}; 

GType gsd_pointingstick_manager_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GSD_POINTINGSTICK_MANAGER_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
