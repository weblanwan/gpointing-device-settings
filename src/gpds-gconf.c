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

#include "gpds-gconf.h"
#include <string.h>

const gchar *
gpds_gconf_get_key_from_path (const gchar *path)
{
    const gchar *segment;

    segment = strrchr(path, '/');

    return segment ? segment + 1 : NULL;
}

gboolean
gpds_gconf_get_bool (GConfClient *gconf, const gchar *key, gboolean *value)
{
    GConfValue *gconf_value;
    gboolean exist_value = FALSE;

    gconf_value = gconf_client_get(gconf, key, NULL);
    if (gconf_value) {
        if (gconf_value->type == GCONF_VALUE_BOOL) {
            *value = gconf_value_get_bool(gconf_value);
            exist_value = TRUE;
        }
        gconf_value_free(gconf_value);
    }

    return exist_value;
}

gboolean
gpds_gconf_get_int (GConfClient *gconf, const gchar *key, gint *value)
{
    GConfValue *gconf_value;
    gboolean exist_value = FALSE;

    gconf_value = gconf_client_get(gconf, key, NULL);
    if (gconf_value) {
        if (gconf_value->type == GCONF_VALUE_INT) {
            *value = gconf_value_get_int(gconf_value);
            exist_value = TRUE;
        }
        gconf_value_free(gconf_value);
    }

    return exist_value;
}

gboolean
gpds_gconf_get_string (GConfClient *gconf, const gchar *key, const gchar **value)
{
    GConfValue *gconf_value;
    gboolean exist_value = FALSE;

    gconf_value = gconf_client_get(gconf, key, NULL);
    if (gconf_value) {
        if (gconf_value->type == GCONF_VALUE_STRING) {
            *value = gconf_value_get_string(gconf_value);
            exist_value = TRUE;
        }
        gconf_value_free(gconf_value);
    }

    return exist_value;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
