#include <gcutter.h>
#include <gtk/gtk.h>
#include "gpds-ui.h"

void gpds_test_warmup (void);
void gpds_test_cooldown (void);

void
gpds_test_warmup (void)
{
    gtk_init(0, NULL);
    gpds_uis_load();
}

void
gpds_test_cooldown (void)
{
    gpds_uis_unload();
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
