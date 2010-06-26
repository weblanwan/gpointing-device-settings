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

#include <gnome-settings-daemon/gnome-settings-plugin.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gdk/gdkx.h>
#include <X11/extensions/XInput.h>

#include "gsd-pointing-device-manager.h"
#include "gpds-gconf.h"
#include "gpds-xinput-pointer-info.h"
#include "gpds-xinput-utils.h"

#define GSD_TYPE_POINTING_DEVICE_PLUGIN            (gsd_pointing_device_plugin_get_type ())
#define GSD_POINTING_DEVICE_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSD_TYPE_POINTING_DEVICE_PLUGIN, GsdPointingDevicePlugin))
#define GSD_POINTING_DEVICE_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSD_TYPE_POINTING_DEVICE_PLUGIN, GsdTracklassPointPluginClass))
#define GSD_IS_POINTING_DEVICE_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSD_TYPE_POINTING_DEVICE_PLUGIN))
#define GSD_IS_POINTING_DEVICE_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSD_TYPE_POINTING_DEVICE_PLUGIN))
#define GSD_POINTING_DEVICE_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSD_TYPE_POINTING_DEVICE_PLUGIN, GsdTracklassPointPluginClass))

typedef struct _GsdPointingDevicePlugin GsdPointingDevicePlugin;
typedef struct _GsdPointingDevicePluginClass GsdPointingDevicePluginClass;

struct _GsdPointingDevicePlugin
{
    GnomeSettingsPlugin parent;
    GList *managers;
};

struct _GsdPointingDevicePluginClass
{
    GnomeSettingsPluginClass parent_class;
}; 

GType gsd_pointing_device_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT GType register_gnome_settings_plugin (GTypeModule *module);

GNOME_SETTINGS_PLUGIN_REGISTER(GsdPointingDevicePlugin, gsd_pointing_device_plugin)

static void
gsd_pointing_device_plugin_init (GsdPointingDevicePlugin *plugin)
{
    plugin->managers = NULL;
}

static gboolean
has_manager (GsdPointingDevicePlugin *plugin, const gchar *device_name)
{
    GList *node;

    for (node = plugin->managers; node; node = g_list_next(node)) {
        GsdPointingDeviceManager *manager = node->data;

        if (g_str_equal(gsd_pointing_device_manager_get_device_name(manager), device_name))
            return TRUE;
    }

    return FALSE;
}

static GdkFilterReturn
device_presence_filter (GdkXEvent *xevent,
                        GdkEvent  *event,
                        gpointer   data)
{
    XEvent *xev = (XEvent *)xevent;
    XEventClass class_presence;
    int xi_presence;
    GsdPointingDevicePlugin *plugin = GSD_POINTING_DEVICE_PLUGIN(data);

    DevicePresence(gdk_x11_get_default_xdisplay(), xi_presence, class_presence);

    if (xev->type == xi_presence) {
        XDeviceInfo *device_info = NULL;
        XDevicePresenceNotifyEvent *notify_event = (XDevicePresenceNotifyEvent *)xev;

        device_info = gpds_xinput_utils_get_device_info_from_id(notify_event->deviceid, NULL);
        if (notify_event->devchange == DeviceEnabled) {
            GsdPointingDeviceManager *manager;

            if (has_manager(plugin, device_info->name))
                return GDK_FILTER_CONTINUE;

            manager = gsd_pointing_device_manager_new(gdk_x11_get_xatom_name(device_info->type),
                                                      device_info->name);
            if (manager) {
                gsd_pointing_device_manager_start(manager, NULL);
                plugin->managers = g_list_prepend(plugin->managers, manager);
            }
        } else if (notify_event->devchange == DeviceRemoved) {
        }
    }

    return GDK_FILTER_CONTINUE;
}

static void
add_device_presence_filter (GsdPointingDevicePlugin *plugin)
{
    Display *display;
    XEventClass class_presence;
    gint xi_presence;

    gint op_code, event, error;

    if (!XQueryExtension(GDK_DISPLAY(),
                         "XInputExtension",
                         &op_code,
                         &event,
                         &error)) {
        return;
    }

    display = gdk_x11_get_default_xdisplay();

    gdk_error_trap_push();
    DevicePresence(display, xi_presence, class_presence);
    XSelectExtensionEvent(display,
                          RootWindow(display, DefaultScreen(display)),
                          &class_presence, 1);
    gdk_flush();
    if (!gdk_error_trap_pop())
        gdk_window_add_filter(NULL, device_presence_filter, plugin);
}

static void
remove_device_presence_filter (GsdPointingDevicePlugin *plugin)
{
    gdk_window_remove_filter(NULL, device_presence_filter, plugin);
}

static void
activate (GnomeSettingsPlugin *plugin)
{
    GsdPointingDevicePlugin *pointing_device_plugin;
    GList *pointer_device_infos, *node;

    pointing_device_plugin = GSD_POINTING_DEVICE_PLUGIN(plugin); 

    add_device_presence_filter(pointing_device_plugin);

    pointer_device_infos = gpds_xinput_utils_collect_pointer_infos();

    for (node = pointer_device_infos; node; node = g_list_next(node)) {
        GsdPointingDeviceManager *manager;
        GpdsXInputPointerInfo *info = node->data;

        manager = gsd_pointing_device_manager_new(gpds_xinput_pointer_info_get_type_name(info),
                                                  gpds_xinput_pointer_info_get_name(info));
        if (!manager)
            continue;

        gsd_pointing_device_manager_start(manager, NULL);
        pointing_device_plugin->managers =
            g_list_prepend(pointing_device_plugin->managers, manager);
    }
    g_list_foreach(pointer_device_infos,
                   (GFunc)gpds_xinput_pointer_info_free, NULL);
    g_list_free(pointer_device_infos);
}

static void
stop_all_managers (GsdPointingDevicePlugin *plugin)
{
    GList *node;

    remove_device_presence_filter(plugin);

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
    stop_all_managers(GSD_POINTING_DEVICE_PLUGIN(plugin));
}

static void
gsd_pointing_device_plugin_class_init (GsdPointingDevicePluginClass *klass)
{
    GnomeSettingsPluginClass *plugin_class = GNOME_SETTINGS_PLUGIN_CLASS(klass);

    plugin_class->activate = activate;
    plugin_class->deactivate = deactivate;
}
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
