gpds_ui_DATA = 		\
	main-window.ui	\
	mouse.ui	\
	touchpad.ui

gpds_icon_DATA = 	\
	mouse.png	\
	trackpoint.png	\
	touchpad.png

@INTLTOOL_SCHEMAS_RULE@

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = 				\
	gpointing-device-settings_gnome_settings_daemon.schemas.in

schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

EXTRA_DIST = 			\
	$(gpds_icon_DATA)	\
	$(schemas_in_files)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$(schemas_DATA)
endif

CLEANFILES = 			\
	$(schemas_DATA)

DISTCLEANFILES =		\
	$(schemas_DATA)

