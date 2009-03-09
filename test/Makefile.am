if WITH_CUTTER
TESTS = run-test.sh
TESTS_ENVIRONMENT = NO_MAKE=yes CUTTER="$(CUTTER)"

noinst_LTLIBRARIES =	                \
	suite_gpds_test.la	        \
	test-gconf.la			\
	test-xinput.la		        \
	test-xinput-pointer-info.la	\
	test-xinput-utils.la	        \
	test-module.la		        \
	test-ui.la                      \
	test-utils.la
endif

EXTRA_DIST =		\
	run-test.sh

INCLUDES =			\
	-I$(srcdir)		\
	-I$(top_srcdir)		\
	-I$(top_srcdir)/src

AM_CFLAGS = 			\
	$(GCUTTER_CFLAGS)	\
	$(GPDS_CFLAGS)

AM_LDFLAGS =			\
	-module			\
	-rpath "`pwd`/.libs"	\
	-avoid-version		\
	-no-undefined

LIBS =					\
	$(top_builddir)/src/libgpds.la	\
	$(GTK_LIBS)			\
	$(GCUTTER_LIBS)

suite_gpds_test_la_SOURCES = gpds-test-suite.c

test_gconf_la_SOURCES = test-gconf.c
test_xinput_la_SOURCES = test-xinput.c
test_xinput_pointer_info_la_SOURCES = test-xinput-pointer-info.c
test_xinput_utils_la_SOURCES = test-xinput-utils.c
test_module_la_SOURCES = test-module.c
test_ui_la_SOURCES = test-ui.c
test_utils_la_SOURCES = test-utils.c

echo-cutter:
	@echo $(CUTTER)
