#include "gpds-module.h"

#include <gcutter.h>

void test_load_modules (void);
void test_collect_names (void);
void test_find (void);

static GList *modules;
static GpdsModule *module;
static GError *error;
static GList *names;
static GList *expected_names;
static GObject *object;

void
cut_startup (void)
{
    modules = gpds_module_load_modules();
}

void
cut_shutdown (void)
{
    g_list_foreach(modules, (GFunc)gpds_module_unload, NULL);
    g_list_free(modules);
    modules = NULL;
}

void
setup (void)
{
    module = NULL;
    names = NULL;
    expected_names = NULL;
    object = NULL;
    error = NULL;
}

void
teardown (void)
{
    if (object)
        g_object_unref(object);
    g_list_free(names);
    g_list_free(expected_names);
}

void
test_collect_names (void)
{
    expected_names = g_list_append(expected_names, "mouse");
    expected_names = g_list_append(expected_names, "pointingstick");
    expected_names = g_list_append(expected_names, "touchpad");

    names = gpds_module_collect_names(modules);

    gcut_assert_equal_list_string(expected_names, names);
}

void
test_find (void)
{
    cut_assert(gpds_module_find(modules, "mouse"));
    cut_assert(gpds_module_find(modules, "touchpad"));
    cut_assert(gpds_module_find(modules, "pointingstick"));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
