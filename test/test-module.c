#include "gpds-module.h"

#include <gcutter.h>

void test_load_modules (void);
void test_collect_names (void);
void test_instantiate (void);

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
    expected_names = g_list_append(expected_names, "touchpad");
    expected_names = g_list_append(expected_names, "trackpoint");

    names = gpds_module_collect_names(modules);

    gcut_assert_equal_list_string(expected_names, names);
}

void
test_instantiate (void)
{
    GList *name;

    cut_trace(test_collect_names());

    for (name = names; name; name = g_list_next(name)) {
        module = gpds_module_find(modules, name->data);
        cut_assert(module);

        object = gpds_module_instantiate(module);
        cut_assert(object);
        g_object_unref(object);
        object = NULL;
    }
}


/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
