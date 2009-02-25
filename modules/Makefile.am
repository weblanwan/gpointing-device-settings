INCLUDES = 			\
	 -I$(top_builddir)	\
	 -I$(top_srcdir)	\
	 -I$(top_srcdir)/src

AM_CFLAGS =			\
	$(GPDS_CFLAGS)		\
	$(COVERAGE_CFLAGS)

AM_LDFLAGS =							\
  -avoid-version -module					\
  -no-undefined -export-dynamic $(LIBTOOL_EXPORT_OPTIONS)

gpds_module_LTLIBRARIES = 	\
	trackpoint.la		\
	touchpad.la

LIBS =					\
	$(top_builddir)/src/libgpds.la	\
	$(GTK_LIBS)

trackpoint_la_SOURCES = gpds-trackpoint-ui.c
touchpad_la_SOURCES = gpds-touchpad-ui.c
