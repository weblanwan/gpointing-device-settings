#include "gpds-utils.h"

#include <gcutter.h>

void test_get_ui_file_directory (void);

static gchar *ui_dir;

void
setup (void)
{
    ui_dir = NULL;
    if (g_getenv("GPDS_UI_DIR")) {
        ui_dir = g_strdup(g_getenv("GPDS_UI_DIR"));
        g_unsetenv("GPDS_UI_DIR");
    }
}

void
teardown (void)
{
    if (ui_dir)
        g_setenv("GPDS_UI_DIR", ui_dir, FALSE);
    g_free(ui_dir);
}

void
test_get_ui_file_directory (void)
{
    cut_assert_equal_string(GPDS_UIDIR,
                            gpds_get_ui_file_directory());
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
