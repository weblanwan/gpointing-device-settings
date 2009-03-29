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

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
