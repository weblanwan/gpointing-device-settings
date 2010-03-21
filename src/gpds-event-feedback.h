/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2010 Hiroyuki Ikezoe  <poincare@ikezoe.net>
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

#ifndef __GPDS_EVENT_FEEDBACK_H__
#define __GPDS_EVENT_FEEDBACK_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPDS_TYPE_EVENT_FEEDBACK            (gpds_event_feedback_get_type ())
#define GPDS_EVENT_FEEDBACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDS_TYPE_EVENT_FEEDBACK, GpdsEventFeedback))
#define GPDS_EVENT_FEEDBACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDS_TYPE_EVENT_FEEDBACK, GpdsEventFeedbackClass))
#define GPDS_IS_EVENT_FEEDBACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDS_TYPE_EVENT_FEEDBACK))
#define GPDS_IS_EVENT_FEEDBACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDS_TYPE_EVENT_FEEDBACK))
#define GPDS_EVENT_FEEDBACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPDS_TYPE_EVENT_FEEDBACK, GpdsEventFeedbackClass))

typedef struct _GpdsEventFeedback GpdsEventFeedback;
typedef struct _GpdsEventFeedbackClass GpdsEventFeedbackClass;

struct _GpdsEventFeedback
{
    GtkWindow parent;
};

struct _GpdsEventFeedbackClass
{
    GtkWindowClass parent_class;
};

GType       gpds_event_feedback_get_type   (void) G_GNUC_CONST;
GtkWidget  *gpds_event_feedback_new        (GtkWindow *parent);

G_END_DECLS

#endif /* __GPDS_EVENT_FEEDBACK_H__ */
/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
