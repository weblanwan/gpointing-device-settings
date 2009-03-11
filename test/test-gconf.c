#include "gpds-gconf.h"

#include <gcutter.h>

void test_get_key_from_path (void);
void test_get_non_existent (void);

static const gchar *key;
static gboolean boolean_value;
static gint int_value;
static const gchar *string_value;
static GConfClient *gconf;

void
setup (void)
{
    key = NULL;
    string_value = NULL;

    gconf = gconf_client_get_default();
}

void
teardown (void)
{
    g_object_unref(gconf);
}

void
test_get_key_from_path (void)
{
    key = gpds_gconf_get_key_from_path("/desktop/gnome/peripherals/TPPS@47@2@32@IBM@32@TrackPoint/middle_button_emulation");
    cut_assert_equal_string("middle_button_emulation", key);
}

void
test_get_non_existent (void)
{
    gchar *unique_key;

    unique_key = gconf_unique_key();
    key = gconf_concat_dir_and_key("/", unique_key);
    g_free(unique_key);

    cut_assert_false(gpds_gconf_get_boolean(gconf, key, &boolean_value));
    cut_assert_false(gpds_gconf_get_int(gconf, key, &int_value));
    cut_assert_false(gpds_gconf_get_string(gconf, key, &string_value));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
