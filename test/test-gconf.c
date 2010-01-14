#include "gpds-gconf.h"

#include <gcutter.h>

void test_get_key_from_path (void);
void test_get_non_existent (void);
void test_boolean (void);
void test_int (void);
void test_float (void);
void test_string (void);

static gchar *gconf_key;
static gboolean boolean_value;
static gint int_value;
static gdouble float_value;
static gchar *string_value;
static GConfClient *gconf;
static GError *error;

void
setup (void)
{
    gconf_key = NULL;
    string_value = NULL;
    error = NULL;

    gconf = gconf_client_get_default();
}

void
teardown (void)
{
    if (gconf_key)
        gconf_client_unset(gconf, gconf_key, NULL);
    g_free(gconf_key);
    g_free(string_value);
    g_object_unref(gconf);
    g_clear_error(&error);
}

void
test_get_key_from_path (void)
{
    const gchar *key;
    key = gpds_gconf_get_key_from_path("/desktop/gnome/peripherals/TPPS@47@2@32@IBM@32@TrackPoint/middle_button_emulation");
    cut_assert_equal_string("middle_button_emulation", key);
}

static gchar *
make_unique_key (void)
{
    const gchar *unique_key;

    unique_key = cut_take_string(gconf_unique_key());
    return gconf_concat_dir_and_key("/", unique_key);
}

void
test_get_non_existent (void)
{
    gconf_key = make_unique_key();

    cut_assert_false(gpds_gconf_get_boolean(gconf, gconf_key, &boolean_value));
    cut_assert_false(gpds_gconf_get_int(gconf, gconf_key, &int_value));
    cut_assert_false(gpds_gconf_get_string(gconf, gconf_key, &string_value));
}

void
test_boolean (void)
{
    gconf_key = make_unique_key();
    gconf_client_set_bool(gconf, gconf_key, TRUE, &error);
    gcut_assert_error(error);

    cut_assert_true(gpds_gconf_get_boolean(gconf, gconf_key, &boolean_value));
    cut_assert_true(boolean_value);
}

void
test_int (void)
{
    gconf_key = make_unique_key();
    gconf_client_set_int(gconf, gconf_key, 99, &error);
    gcut_assert_error(error);

    cut_assert_true(gpds_gconf_get_int(gconf, gconf_key, &int_value));
    cut_assert_equal_int(99, int_value);
}

void
test_float (void)
{
    gconf_key = make_unique_key();
    gconf_client_set_float(gconf, gconf_key, 0.99, &error);
    gcut_assert_error(error);

    cut_assert_true(gpds_gconf_get_float(gconf, gconf_key, &float_value));
    cut_assert_equal_double(0.99, 0.0001, float_value);
}

void
test_string (void)
{
    gconf_key = make_unique_key();
    gconf_client_set_string(gconf, gconf_key, "string", &error);
    gcut_assert_error(error);

    cut_assert_true(gpds_gconf_get_string(gconf, gconf_key, &string_value));
    cut_assert_equal_string("string", string_value);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
