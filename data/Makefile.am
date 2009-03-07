gpds_ui_DATA = 		\
	mouse.ui	\
	touchpad.ui

gpds_icon_DATA = 	\
	trackpoint.png	\
	touchpad.png

@INTLTOOL_SCHEMAS_RULE@

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = 				\
	gpointing-device-settings_gnome_settings_daemon.schemas.in

schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

EXTRA_DIST = 			\
	mouse.ui.in	        \
	touchpad.ui.in		\
	$(gpds_icon_DATA)	\
	$(schemas_in_files)

CLEANFILES = 			\
	$(schemas_DATA)

DISTCLEANFILES =		\
	$(schemas_DATA)

