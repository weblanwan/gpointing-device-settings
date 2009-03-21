lib_LTLIBRARIES = libgpds.la

libgpds_public_headers = 		\
	gpointing-device-settings.h	\
	gpds-module-impl.h		\
	gpds-gconf.h			\
	gpds-utils.h			\
	gpds-ui.h			\
	gpds-xinput.h			\
	gpds-xinput-pointer-info.h	\
	gpds-xinput-utils.h

libgpds_la_SOURCES =			\
	$(libgpds_public_headers)	\
	gpds-module.c			\
	gpds-module.h			\
	gpds-ui.c			\
	gpds-gconf.c			\
	gpds-utils.c			\
	gpds-xinput.c			\
	gpds-xinput-pointer-info.c	\
	gpds-xinput-utils.c

AM_CFLAGS = -DG_LOG_DOMAIN=\"GPointingDeviceSettings\"

libgpds_la_CFLAGS =			\
	$(GPDS_CFLAGS)			\
	$(COVERAGE_CFLAGS)

libgpds_la_LDFLAGS =				\
	-version-info $(LT_VERSION_INFO)	\
	-no-undefined

libgpds_la_LIBADD =			\
	$(GTK_LIBS)			\
	$(GCONF2_LIBS)			\
	$(XINPUT_LIBS)

pkginclude_HEADERS =			\
	$(libgpds_public_headers)

bin_PROGRAMS = gpointing-device-settings

gpointing_device_settings_CFLAGS =	\
	-DLOCALEDIR=\""$(localedir)"\"	\
	$(GPDS_CFLAGS)			\
	$(COVERAGE_CFLAGS)

gpointing_device_settings_SOURCES =	\
	gpointing-device-settings.c

gpointing_device_settings_LDADD =	\
	libgpds.la			\
	$(GTK_LIBS)			\
	$(XINPUT_LIBS)
