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

#ifndef __GSD_MOUSE_EXTENSION_MANAGER_H__
#define __GSD_MOUSE_EXTENSION_MANAGER_H__

#include "gsd-pointing-device-manager.h"

G_BEGIN_DECLS

#define GSD_TYPE_MOUSE_EXTENSION_MANAGER            (gsd_mouse_extension_manager_get_type ())
#define GSD_MOUSE_EXTENSION_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSD_TYPE_MOUSE_EXTENSION_MANAGER, GsdMouseExtensionManager))
#define GSD_MOUSE_EXTENSION_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSD_TYPE_MOUSE_EXTENSION_MANAGER, GsdTracklassPointManagerClass))
#define GSD_IS_MOUSE_EXTENSION_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSD_TYPE_MOUSE_EXTENSION_MANAGER))
#define GSD_IS_MOUSE_EXTENSION_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSD_TYPE_MOUSE_EXTENSION_MANAGER))
#define GSD_MOUSE_EXTENSION_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSD_TYPE_MOUSE_EXTENSION_MANAGER, GsdTracklassPointManagerClass))

typedef struct _GsdMouseExtensionManager GsdMouseExtensionManager;
typedef struct _GsdMouseExtensionManagerClass GsdMouseExtensionManagerClass;

struct _GsdMouseExtensionManager
{
    GsdPointingDeviceManager parent;
};

struct _GsdMouseExtensionManagerClass
{
    GsdPointingDeviceManagerClass parent_class;
}; 

GType gsd_mouse_extension_manager_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GSD_MOUSE_EXTENSION_MANAGER_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
