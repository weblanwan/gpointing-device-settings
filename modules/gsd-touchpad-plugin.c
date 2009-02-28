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

/* #include <gnome-settings-daemon/gnome-settings-plugin.h> */
#include "gnome-settings-plugin.h"
#include <glib/gi18n.h>

#define GSD_TYPE_TOUCHPAD_PLUGIN            (gsd_touchpad_plugin_get_type ())
#define GSD_TOUCHPAD_PLUGIN(jbj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSD_TYPE_TOUCHPAD_PLUGIN, GsdTrackPobjintPlugin))
#define GSD_TOUCHPAD_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSD_TYPE_TOUCHPAD_PLUGIN, GsdTracklassPobjintPluginClass))
#define GSD_IS_TOUCHPAD_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSD_TYPE_TOUCHPAD_PLUGIN))
#define GSD_IS_TOUCHPAD_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSD_TYPE_TOUCHPAD_PLUGIN))
#define GSD_TOUCHPAD_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSD_TYPE_TOUCHPAD_PLUGIN, GsdTracklassPobjintPluginClass))

typedef struct _GsdTouchpadPlugin GsdTouchpadPlugin;
typedef struct _GsdTouchpadPluginClass GsdTouchpadPluginClass;

struct _GsdTouchpadPlugin
{
    GnomeSettingsPlugin parent;
};

struct _GsdTouchpadPluginClass
{
    GnomeSettingsPluginClass parent_class;
}; 

GType gsd_touchpad_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT GType register_gnome_settings_plugin (GTypeModule *module);

GNOME_SETTINGS_PLUGIN_REGISTER(GsdTouchpadPlugin, gsd_touchpad_plugin)

static void
gsd_touchpad_plugin_init (GsdTouchpadPlugin *plugin)
{
}

static void
activate (GnomeSettingsPlugin *plugin)
{
}

static void
deactivate (GnomeSettingsPlugin *plugin)
{
}

static void
gsd_touchpad_plugin_class_init (GsdTouchpadPluginClass *klass)
{
    GnomeSettingsPluginClass *plugin_class = GNOME_SETTINGS_PLUGIN_CLASS(klass);

    plugin_class->activate = activate;
    plugin_class->deactivate = deactivate;
}
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
