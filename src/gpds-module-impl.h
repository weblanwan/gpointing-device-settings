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

#ifndef __GPDS_MODULE_IMPL_H__
#define __GPDS_MODULE_IMPL_H__

#include <glib-object.h>

G_BEGIN_DECLS

#include "gpds-module.h"

typedef GList   *(*GpdsModuleInitFunc)         (GTypeModule *module);
typedef void     (*GpdsModuleExitFunc)         (void);
typedef GObject *(*GpdsModuleInstantiateFunc)  (void);

#define GPDS_MODULE_IMPL_INIT           gpds_module_impl_init
#define GPDS_MODULE_IMPL_EXIT           gpds_module_impl_exit
#define GPDS_MODULE_IMPL_INSTANTIATE    gpds_module_impl_instantiate

GList   *GPDS_MODULE_IMPL_INIT           (GTypeModule  *module);
void     GPDS_MODULE_IMPL_EXIT           (void);
GObject *GPDS_MODULE_IMPL_INSTANTIATE    (void);

G_END_DECLS

#endif /* __GPDS_MODULE_IMPL_H__ */

/*
vi:nowrap:ai:expandtab:sw=4
*/
