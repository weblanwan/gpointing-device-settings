SUBDIRS = common gnome-settings-daemon-plugins

INCLUDES = 			        \
	 -I$(top_builddir)	        \
	 -I$(top_srcdir)	        \
	 -I$(top_srcdir)/src            \
	 -I$(top_srcdir)/modules/common

AM_CFLAGS =			\
	$(GPDS_CFLAGS)		\
	$(GCONF2_CFLAGS)	\
	$(COVERAGE_CFLAGS)

AM_LDFLAGS =			\
  	-avoid-version -module	\
  	-no-undefined 		\
  	-export-dynamic $(LIBTOOL_EXPORT_OPTIONS)

gpds_module_LTLIBRARIES = 	\
	trackpoint.la		\
	touchpad.la

LIBS =					\
	$(top_builddir)/src/libgpds.la	\
	$(GTK_LIBS)			\
	$(GCONF2_LIBS)			\
	$(GNOME_SETTINGS_DAEMON_LIBS)

trackpoint_la_SOURCES = 		\
	gpds-trackpoint-ui.c

trackpoint_la_LIBADD = $(top_builddir)/modules/common/libmouse_common.la

touchpad_la_SOURCES = 			\
	gpds-touchpad-ui.c

touchpad_la_LIBADD = $(top_builddir)/modules/common/libtouchpad_common.la
