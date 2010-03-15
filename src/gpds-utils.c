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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "gpds-utils.h"

const gchar *
gpds_get_ui_file_directory (void)
{
    const gchar *dir;

    dir = g_getenv("GPDS_UI_DIR");
    return dir ? dir : GPDS_UIDIR;
}

const gchar *
gpds_get_icon_file_directory (void)
{
    const gchar *dir;

    dir = g_getenv("GPDS_ICON_DIR");
    return dir ? dir : GPDS_ICONDIR;
}

GdkPixbuf *
gpds_convert_to_grayscaled_pixbuf (GdkPixbuf *src)
{
    GdkPixbuf *dest;
    gint width, height;
    guchar *pixels;
    int rowstride, n_channels;
    int x, y;

    dest = gdk_pixbuf_copy(src);
    width = gdk_pixbuf_get_width(dest);
    height = gdk_pixbuf_get_height(dest);
    rowstride = gdk_pixbuf_get_rowstride(dest);
    n_channels = gdk_pixbuf_get_n_channels(dest);
    pixels = gdk_pixbuf_get_pixels(dest);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width * n_channels; x += n_channels) {
            guchar grayscale;
            guchar *p;

            p = pixels + y * rowstride + x;
            grayscale = (p[0] * 11 + p[1] * 16 + p[2] * 5) / 32;
            p[0] = grayscale;
            p[1] = grayscale;
            p[2] = grayscale;
        }
    }

    return dest;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
