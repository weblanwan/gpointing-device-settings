gpds_ui_DATA = 		\
	trackpoint.ui	\
	touchpad.ui

gpds_icon_DATA = 	\
	trackpoint.png	\
	touchpad.png

@INTLTOOL_SCHEMAS_RULE@

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = 				\
	gnome-settings-daemon.schemas.in

schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

EXTRA_DIST = 			\
	trackpoint.ui.in	\
	touchpad.ui.in		\
	$(gpds_icon_DATA)	\
	$(schemas_in_files)

