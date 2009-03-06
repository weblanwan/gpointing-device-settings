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

#include "gsd-mouse-extension-manager.h"

#define GSD_TYPE_MOUSE_EXTENSION_PLUGIN            (gsd_mouse_extension_plugin_get_type ())
#define GSD_MOUSE_EXTENSION_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSD_TYPE_MOUSE_EXTENSION_PLUGIN, GsdMouseExtensionPlugin))
#define GSD_MOUSE_EXTENSION_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSD_TYPE_MOUSE_EXTENSION_PLUGIN, GsdTracklassPointPluginClass))
#define GSD_IS_MOUSE_EXTENSION_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSD_TYPE_MOUSE_EXTENSION_PLUGIN))
#define GSD_IS_MOUSE_EXTENSION_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSD_TYPE_MOUSE_EXTENSION_PLUGIN))
#define GSD_MOUSE_EXTENSION_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSD_TYPE_MOUSE_EXTENSION_PLUGIN, GsdTracklassPointPluginClass))

typedef struct _GsdMouseExtensionPlugin GsdMouseExtensionPlugin;
typedef struct _GsdMouseExtensionPluginClass GsdMouseExtensionPluginClass;

struct _GsdMouseExtensionPlugin
{
    GnomeSettingsPlugin parent;
    GsdMouseExtensionManager *manager;
};

struct _GsdMouseExtensionPluginClass
{
    GnomeSettingsPluginClass parent_class;
}; 

GType gsd_mouse_extension_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT GType register_gnome_settings_plugin (GTypeModule *module);

GNOME_SETTINGS_PLUGIN_REGISTER(GsdMouseExtensionPlugin, gsd_mouse_extension_plugin)

static void
gsd_mouse_extension_plugin_init (GsdMouseExtensionPlugin *plugin)
{
    plugin->manager = NULL;
}

static void
activate (GnomeSettingsPlugin *plugin)
{
    GsdMouseExtensionPlugin *mouse_extension_plugin;

    mouse_extension_plugin = GSD_MOUSE_EXTENSION_PLUGIN(plugin); 
    mouse_extension_plugin->manager = gsd_mouse_extension_manager_new();
    gsd_mouse_extension_manager_start(mouse_extension_plugin->manager, NULL);
}

static void
deactivate (GnomeSettingsPlugin *plugin)
{
    GsdMouseExtensionPlugin *mouse_extension_plugin;

    mouse_extension_plugin = GSD_MOUSE_EXTENSION_PLUGIN(plugin); 
    if (mouse_extension_plugin->manager) {
        gsd_mouse_extension_manager_stop(mouse_extension_plugin->manager);
        g_object_unref(mouse_extension_plugin->manager);
        mouse_extension_plugin->manager = NULL;
    }
}

static void
gsd_mouse_extension_plugin_class_init (GsdMouseExtensionPluginClass *klass)
{
    GnomeSettingsPluginClass *plugin_class = GNOME_SETTINGS_PLUGIN_CLASS(klass);

    plugin_class->activate = activate;
    plugin_class->deactivate = deactivate;
}
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
