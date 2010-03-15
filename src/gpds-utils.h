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

#ifndef __GPDS_UTILS_H__
#define __GPDS_UTILS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

const gchar *gpds_get_ui_file_directory (void);
const gchar *gpds_get_icon_file_directory (void);
GdkPixbuf   *gpds_convert_to_grayscaled_pixbuf (GdkPixbuf *src);

G_END_DECLS

#endif /* __GPDS_UTILS_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/

