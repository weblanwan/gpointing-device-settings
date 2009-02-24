if WITH_CUTTER
TESTS = run-test.sh
TESTS_ENVIRONMENT = NO_MAKE=yes CUTTER="$(CUTTER)"

noinst_LTLIBRARIES =			\
	suite_gpointing-device-settings_test.la	\
	test_xinput.la	
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

LIBS =				\
	$(GTK_LIBS)		\
	$(GCUTTER_LIBS)

suite_gpointing-device-settings_test_la_SOURCES = gpointing-device-settings-test-suite.c

test_xinput_la_SOURCES = test-xinput.c
test_xinput_la_LIBADD = $(top_builddir)/src/gpointing-device-settings-gxinput.o

echo-cutter:
	@echo $(CUTTER)
