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
#include <gconf/gconf-client.h>

#include "gsd-pointing-device-manager.h"
#include "gpds-gconf.h"
#include "gpds-xinput-pointer-info.h"

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
    GList *managers;
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
    plugin->managers = NULL;
}

static GList *
collect_pointer_device_infos_from_gconf (void)
{
    GConfClient *gconf;
    GSList *dirs, *node;
    GList *infos = NULL;

    gconf = gconf_client_get_default();
    dirs = gconf_client_all_dirs(gconf, GPDS_GCONF_DIR, NULL);

    for (node = dirs; node; node = g_slist_next(node)) {
        const gchar *dir = node->data;
        gchar *device_type;
        gchar *device_type_key;

        device_type_key = gconf_concat_dir_and_key(dir, GPDS_GCONF_DEVICE_TYPE_KEY);
        device_type = gconf_client_get_string(gconf, device_type_key, NULL);
        if (device_type && !strcmp(device_type, "mouse")) {
            GpdsXInputPointerInfo *info;
            gchar *device_name;

            device_name = g_path_get_basename(dir);
            info = gpds_xinput_pointer_info_new(device_name, device_type);
            infos = g_list_prepend(infos, info);
            g_free(device_name);
        }

        g_free(device_type_key);
        g_free(device_type);
    }

    g_slist_foreach(dirs, (GFunc)g_free, NULL);
    g_slist_free(dirs);
    g_object_unref(gconf);

    return infos;
}

static void
activate (GnomeSettingsPlugin *plugin)
{
    GsdMouseExtensionPlugin *mouse_extension_plugin;
    GList *pointer_device_infos, *node;

    mouse_extension_plugin = GSD_MOUSE_EXTENSION_PLUGIN(plugin); 

    pointer_device_infos = collect_pointer_device_infos_from_gconf();
    for (node = pointer_device_infos; node; node = g_list_next(node)) {
        GsdPointingDeviceManager *manager;
        GpdsXInputPointerInfo *info = node->data;

        manager = gsd_pointing_device_manager_new(gpds_xinput_pointer_info_get_type_name(info),
                                                  gpds_xinput_pointer_info_get_name(info));
        if (!manager)
            continue;

        gsd_pointing_device_manager_start(manager, NULL);
        mouse_extension_plugin->managers =
            g_list_prepend(mouse_extension_plugin->managers, manager);
    }
    g_list_foreach(pointer_device_infos,
                   (GFunc)gpds_xinput_pointer_info_free, NULL);
    g_list_free(pointer_device_infos);
}

static void
stop_all_managers (GsdMouseExtensionPlugin *plugin)
{
    GList *node;

    for (node = plugin->managers; node; node = g_list_next(node)) {
        GsdPointingDeviceManager *manager = node->data;

        gsd_pointing_device_manager_stop(manager);
        g_object_unref(manager);
    }

    g_list_free(plugin->managers);
    plugin->managers = NULL;
}

static void
deactivate (GnomeSettingsPlugin *plugin)
{
    stop_all_managers(GSD_MOUSE_EXTENSION_PLUGIN(plugin));
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
