dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)

AC_INIT(gpointing-device-settings, 1.0.0)
AM_INIT_AUTOMAKE([1.9 tar-pax])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_PROG_LIBTOOL

AC_PATH_PROG([PKG_CONFIG], [pkg-config], [no])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_INTLTOOL([0.35.0])
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=gpointing-device-settings
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext package])

AM_GLIB_GNU_GETTEXT

if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wmissing-declarations[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wmissing-declarations" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wmissing-prototypes[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wmissing-prototypes" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wpointer-arith[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wpointer-arith" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wcast-align[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wcast-align" ;;
  esac

  if test "x$enable_ansi" = "xyes"; then
    case " $CFLAGS " in
    *[\ \	]-ansi[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -ansi" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-pedantic[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -pedantic" ;;
    esac
  fi
fi

GTK_REQUIRED=2.14.0
AC_SUBST(GTK_REQUIRED)
AM_PATH_GTK_2_0($GTK_REQUIRED)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)

dnl PKG_CHECK_MODULES(GNOME_SETTINGS_DAEMON, gnome-settings-daemon >= 2.24.0)
dnl AC_SUBST(GNOME_SETTINGS_DAEMON_CFLAGS)
dnl AC_SUBST(GNOME_SETTINGS_DAEMON_LIBS)

GCONF_REQUIRED=2.24.0
AC_SUBST(GCONF_REQUIRED)
PKG_CHECK_MODULES(GCONF2, gconf-2.0 >= $GCONF_REQUIRED)
AC_SUBST(GCONF2_CFLAGS)
AC_SUBST(GCONF2_LIBS)
AM_GCONF_SOURCE_2

PKG_CHECK_MODULES(XINPUT, xi >= 1.2.0)
AC_SUBST(XINPUT_CFLAGS)
AC_SUBST(XINPUT_LIBS)

LT_CURRENT=1
LT_REVISION=0
LT_AGE=1
LT_VERSION_INFO="$LT_CURRENT:$LT_REVISION:$LT_AGE"
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_VERSION_INFO)

LT_CURRENT_MINUS_AGE=`expr $LT_CURRENT - $LT_AGE`
AC_SUBST(LT_CURRENT_MINUS_AGE)

LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

dnl **************************************************************
dnl Checks for typedefs, structures, and compiler characteristics.
dnl **************************************************************
AC_C_CONST

gpds_moduledir="\${libdir}/\$(PACKAGE)/module"
AC_SUBST(gpds_moduledir)
gpds_datadir="\${datadir}/\$(PACKAGE)"
AC_SUBST(gpds_datadir)
gpds_uidir="\${datadir}/\$(PACKAGE)/ui"
AC_SUBST(gpds_uidir)
gpds_icondir="\${datadir}/\$(PACKAGE)/icon"
AC_SUBST(gpds_icondir)
gpds_help_datadir="\${datadir}"
AC_SUBST(gpds_help_datadir)

gsd_plugindir="\${libdir}/gnome-settings-daemon-2.0"
AC_SUBST([gsd_plugindir])

GSD_INTLTOOL_PLUGIN_RULE='%.gnome-settings-plugin:   %.gnome-settings-plugin.in   $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< [$]@'
AC_SUBST([GSD_INTLTOOL_PLUGIN_RULE])

dnl **************************************************************
dnl Check for Cutter
dnl **************************************************************
m4_ifdef([AC_CHECK_GCUTTER], [
AC_CHECK_GCUTTER(>= 1.0.6)
],
[ac_cv_use_cutter="no"])

AM_CONDITIONAL([WITH_CUTTER], [test "$ac_cv_use_cutter" != "no"])
if test "$ac_cv_use_cutter" != "no"; then
  AC_DEFINE(WITH_CUTTER, 1, [Define to 1 if you use Cutter])
fi

m4_ifdef([AC_CHECK_COVERAGE], [AC_CHECK_COVERAGE])
GENHTML_OPTIONS="--title 'GPointingDeviceSettings Code Coverage'"

GPDS_CFLAGS="$XINPUT_CFLAGS $GTK_CFLAGS"
GPDS_CFLAGS="$GPDS_CFLAGS -DGTK_DISABLE_DEPRECATED"
GPDS_CFLAGS="$GPDS_CFLAGS -DGDK_DISABLE_DEPRECATED"
GPDS_CFLAGS="$GPDS_CFLAGS -DG_DISABLE_DEPRECATED"
GPDS_CFLAGS="$GPDS_CFLAGS -DGPDS_MODULEDIR=\\\"\"\$(gpds_moduledir)\"\\\""
GPDS_CFLAGS="$GPDS_CFLAGS -DGPDS_DATADIR=\\\"\"\$(gpds_datadir)\"\\\""
GPDS_CFLAGS="$GPDS_CFLAGS -DGPDS_UIDIR=\\\"\"\$(gpds_uidir)\"\\\""
GPDS_CFLAGS="$GPDS_CFLAGS -DGPDS_ICONDIR=\\\"\"\$(gpds_icondir)\"\\\""
GPDS_CFLAGS="$GPDS_CFLAGS -DGPDS_HELP_DATADIR=\\\"\"\$(gpds_help_datadir)\"\\\""
AC_SUBST(GPDS_CFLAGS)

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
src/Makefile
modules/Makefile
modules/common/Makefile
modules/gnome-settings-daemon-plugins/Makefile
data/Makefile
data/mouse.ui
data/touchpad.ui
test/Makefile
po/Makefile.in
libgpointing-device-settings.pc
])

AC_OUTPUT
