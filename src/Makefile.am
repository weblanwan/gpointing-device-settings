bin_PROGRAMS = gpointing-device-settings

gpointing_device_settings_CFLAGS =		\
	-DDATADIR=\""$(datadir)/$(PACKAGE)"\" 	\
	-DHELPDATADIR=\""$(datadir)"\"	 	\
	$(XINPUT_CFLAGS)			\
	$(GTK_CFLAGS)

gpointing_device_settings_SOURCES =	\
	gpointing-device-settings.c	\
	gtrackpoint-ui.c		\
	gtrackpoint-ui.h		\
	gxinput.c			\
	gxinput.h

gpointing_device_settings_LDADD =	\
	$(GTK_LIBS)	\
	$(XINPUT_LIBS)
