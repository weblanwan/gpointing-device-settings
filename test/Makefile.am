if WITH_CUTTER
TESTS = run-test.sh
TESTS_ENVIRONMENT = NO_MAKE=yes CUTTER="$(CUTTER)"

noinst_LTLIBRARIES =	\
	suite_gpds_test.la	\
	test-xinput.la		\
	test-module.la		\
	test-ui.la
endif

EXTRA_DIST =		\
	run-test.sh

INCLUDES =			\
	-I$(srcdir)		\
	-I$(top_srcdir)		\
	-I$(top_srcdir)/src

AM_CFLAGS = 			\
	$(GCUTTER_CFLAGS)	\
	$(GTK_CFLAGS)

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

test_xinput_la_SOURCES = test-xinput.c
test_module_la_SOURCES = test-module.c
test_ui_la_SOURCES = test-ui.c

echo-cutter:
	@echo $(CUTTER)
