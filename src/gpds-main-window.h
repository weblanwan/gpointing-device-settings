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

#ifndef __GPDS_MAIN_WINDOW_H__
#define __GPDS_MAIN_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPDS_TYPE_MAIN_WINDOW            (gpds_main_window_get_type ())
#define GPDS_MAIN_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_MAIN_WINDOW, GpdsMainWindow))
#define GPDS_MAIN_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_MAIN_WINDOW, GpdsMainWindowClass))
#define GPDS_IS_MAIN_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_MAIN_WINDOW))
#define GPDS_IS_MAIN_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_MAIN_WINDOW))
#define GPDS_MAIN_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_MAIN_WINDOW, GpdsMainWindowClass))

typedef struct _GpdsMainWindow GpdsMainWindow;
typedef struct _GpdsMainWindowClass GpdsMainWindowClass;

struct _GpdsMainWindow
{
    GtkDialog parent;
};

struct _GpdsMainWindowClass
{
    GtkDialogClass parent_class;
};

GType       gpds_main_window_get_type   (void) G_GNUC_CONST;
GtkWidget  *gpds_main_window_new        (void);

G_END_DECLS

#endif /* __GPDS_MAIN_WINDOW_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
