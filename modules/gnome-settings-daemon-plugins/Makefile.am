@GSD_INTLTOOL_PLUGIN_RULE@

INCLUDES = 				\
	 -I$(top_builddir)		\
	 -I$(top_srcdir)		\
	 -I$(top_srcdir)/src		\
	 -I$(top_srcdir)/modules/common

AM_CFLAGS =			\
	$(GPDS_CFLAGS)		\
	$(GCONF2_CFLAGS)	\
	$(COVERAGE_CFLAGS)

AM_LDFLAGS =			\
  	-avoid-version -module	\
  	-no-undefined 		\
  	-export-dynamic $(LIBTOOL_EXPORT_OPTIONS)

LIBS =					\
	$(top_builddir)/src/libgpds.la	\
	$(GTK_LIBS)			\
	$(GCONF2_LIBS)			\
	$(GNOME_SETTINGS_DAEMON_LIBS)

gsd_plugin_LTLIBRARIES =		\
	libpointing-device.la

gsd_plugin_CFLAGS =					\
	$(AM_CFLAGS)					\
	-DG_LOG_DOMAIN=\"GPDS-Plugins\"			\
	$(GNOME_SETTINGS_DAEMON_CFLAGS)			\
	-DGNOME_SETTINGS_LOCALEDIR=\""$(localedir)"\"

libpointing_device_la_CFLAGS = $(gsd_plugin_CFLAGS)
libpointing_device_la_SOURCES = 	\
	gsd-pointing-device-plugin.c	\
	gsd-pointing-device-manager.h	\
	gsd-pointing-device-manager.c	\
	gsd-mouse-extension-manager.h	\
	gsd-mouse-extension-manager.c	\
	gsd-touchpad-manager.h		\
	gsd-touchpad-manager.c

libpointing_device_la_LIBADD =					\
        $(top_builddir)/modules/common/libmouse_common.la	\
        $(top_builddir)/modules/common/libtouchpad_common.la

gsd_plugin_in_files =					\
	pointing-device.gnome-settings-plugin.in

gsd_plugin_DATA = $(gsd_plugin_in_files:.gnome-settings-plugin.in=.gnome-settings-plugin)

EXTRA_DIST = 			\
	$(gsd_plugin_in_files)	\
	gnome-settings-plugin.h

CLEANFILES = 			\
	$(gsd_plugin_DATA)

DISTCLEANFILES =		\
	$(gsd_plugin_DATA)

