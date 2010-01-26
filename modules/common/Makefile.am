INCLUDES = 			\
	 -I$(top_builddir)	\
	 -I$(top_srcdir)	\
	 -I$(top_srcdir)/src

AM_CFLAGS =			\
	$(GPDS_CFLAGS)		\
	$(GCONF2_CFLAGS)	\
	$(COVERAGE_CFLAGS)

noinst_LTLIBRARIES =            	\
        libpointingstick_common.la	\
        libtouchpad_common.la		\
        libmouse_common.la

LIBADD =				\
	$(top_builddir)/src/libgpds.la	\
	$(GTK_LIBS)			\
	$(GCONF2_LIBS)			\
	$(GNOME_SETTINGS_DAEMON_LIBS)

libmouse_common_la_SOURCES =       \
        gpds-mouse-xinput.c        \
        gpds-mouse-xinput.h        \
	gpds-mouse-definitions.h

libtouchpad_common_la_SOURCES =         \
        gpds-touchpad-xinput.c          \
        gpds-touchpad-xinput.h          \
	gpds-touchpad-definitions.h
        
libpointingstick_common_la_SOURCES =       \
        gpds-pointingstick-xinput.c        \
        gpds-pointingstick-xinput.h        \
	gpds-pointingstick-definitions.h

