#include "gpds-gconf.h"

#include <gcutter.h>

void test_get_key_from_path (void);

static const gchar *key;

void
setup (void)
{
    key = NULL;
}

void
teardown (void)
{
}

void
test_get_key_from_path (void)
{
    key = gpds_gconf_get_key_from_path("/desktop/gnome/peripherals/TPPS@47@2@32@IBM@32@TrackPoint/middle_button_emulation");
    cut_assert_equal_string("middle_button_emulation", key);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
