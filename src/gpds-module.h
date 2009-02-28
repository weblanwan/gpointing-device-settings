/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __GPDS_MODULE_H__
#define __GPDS_MODULE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GPDS_TYPE_MODULE            (gpds_module_get_type ())
#define GPDS_MODULE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_MODULE, GpdsModule))
#define GPDS_MODULE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_MODULE, GpdsModuleClass))
#define GPDS_IS_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_MODULE))
#define GPDS_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_MODULE))
#define GPDS_MODULE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_MODULE, GpdsModuleClass))

typedef struct _GpdsModule GpdsModule;
typedef struct _GpdsModuleClass GpdsModuleClass;

struct _GpdsModule
{
    GTypeModule object;
};

struct _GpdsModuleClass
{
    GTypeModuleClass parent_class;
};

GType        gpds_module_get_type            (void) G_GNUC_CONST;

GpdsModule  *gpds_module_load_module         (const gchar    *base_dir,
                                              const gchar    *name);
GList       *gpds_module_load_modules        (void);
GList       *gpds_module_load_modules_unique (const gchar    *base_dir,
                                              GList          *modules);
GpdsModule  *gpds_module_find                (GList          *modules,
                                              const gchar    *name);
const gchar *gpds_module_directory           (void);

GList       *gpds_module_collect_names            (GList *modules);

GObject     *gpds_module_instantiate              (GpdsModule *module);
void         gpds_module_unload                   (GpdsModule *module);


G_END_DECLS

#endif /* __GPDS_MODULE_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
