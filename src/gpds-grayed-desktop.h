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

#ifndef __GPDS_GRAYED_DESKTOP_H__
#define __GPDS_GRAYED_DESKTOP_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPDS_TYPE_GRAYED_DESKTOP            (gpds_grayed_desktop_get_type ())
#define GPDS_GRAYED_DESKTOP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_GRAYED_DESKTOP, GpdsGrayedDesktop))
#define GPDS_GRAYED_DESKTOP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_GRAYED_DESKTOP, GpdsGrayedDesktopClass))
#define GPDS_IS_GRAYED_DESKTOP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_GRAYED_DESKTOP))
#define GPDS_IS_GRAYED_DESKTOP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_GRAYED_DESKTOP))
#define GPDS_GRAYED_DESKTOP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_GRAYED_DESKTOP, GpdsGrayedDesktopClass))

typedef struct _GpdsGrayedDesktop GpdsGrayedDesktop;
typedef struct _GpdsGrayedDesktopClass GpdsGrayedDesktopClass;

struct _GpdsGrayedDesktop
{
    GtkWindow parent;
};

struct _GpdsGrayedDesktopClass
{
    GtkWindowClass parent_class;
};

GType       gpds_grayed_desktop_get_type (void) G_GNUC_CONST;
GtkWidget  *gpds_grayed_desktop_new      (GtkWindow *main_window);

G_END_DECLS

#endif /* __GPDS_GRAYED_DESKTOP_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
