bin_PROGRAMS = gpointing-device-settings

g_pointing-device-settings_CFLAGS =		\
	-DDATADIR=\""$(datadir)/$(PACKAGE)"\" 	\
	-DHELPDATADIR=\""$(datadir)"\"	 	\
	$(XINPUT_CFLAGS)			\
	$(GTK_CFLAGS)

gpointing-device-settings_SOURCES =	\
	gpointing-device-settings.c	\
	gtrackpoint-ui.c		\
	gtrackpoint-ui.h		\
	gxinput.c			\
	gxinput.h

gpointing-device-settings_LDADD =	\
	$(GTK_LIBS)	\
	$(XINPUT_LIBS)
