INCLUDES = 			\
	 -I$(top_builddir)	\
	 -I$(top_srcdir)	\
	 -I$(top_srcdir)/src

AM_CFLAGS =			\
	$(GPDS_CFLAGS)		\
	$(GCONF2_CFLAGS)	\
	$(COVERAGE_CFLAGS)

noinst_LTLIBRARIES =            \
        libtrackpoint_common.la \
        libtouchpad_common.la

LIBADD =				\
	$(top_builddir)/src/libgpds.la	\
	$(GTK_LIBS)			\
	$(GCONF2_LIBS)			\
	$(GNOME_SETTINGS_DAEMON_LIBS)

libtrackpoint_common_la_SOURCES =       \
        gpds-trackpoint-xinput.c        \
        gpds-trackpoint-xinput.h        \
	gpds-trackpoint-definitions.h

libtouchpad_common_la_SOURCES =         \
        gpds-touchpad-xinput.c          \
        gpds-touchpad-xinput.h          \
	gpds-touchpad-definitions.h
        
