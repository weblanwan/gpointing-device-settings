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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <math.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gpds-main-window.h"
#include "gpds-utils.h"
#include "gpds-ui.h"
#include "gpds-xinput-pointer-info.h"
#include "gpds-ui.h"
#include "gpds-utils.h"
#include "gpds-grayed-desktop.h"
#include "gpds-event-feedback.h"


enum
{
    PROP_0,
    PROP_PARENT
};

typedef struct _GpdsEventFeedbackPriv GpdsEventFeedbackPriv;
struct _GpdsEventFeedbackPriv
{
    GtkWidget *parent;
    GtkImage *image;
    GdkPixbufAnimation *buttons[5];
    GdkPixbufAnimation *scrollings[4];
    guint scroll_timeout_id;
    gboolean is_scrolling;
    GdkScrollDirection scroll_direction;
    gboolean is_finished_scrolling;
};

#define GPDS_EVENT_FEEDBACK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_EVENT_FEEDBACK, GpdsEventFeedbackPriv))

G_DEFINE_TYPE(GpdsEventFeedback, gpds_event_feedback, GTK_TYPE_WINDOW)

static GObject *constructor    (GType             type,
                                guint             n_props,
                                GObjectConstructParam
                                                 *props);
static void     dispose        (GObject          *object);
static void     set_property   (GObject          *object,
                                guint             prop_id,
                                const GValue     *value,
                                GParamSpec       *pspec);
static void     get_property   (GObject          *object,
                                guint             prop_id,
                                GValue           *value,
                                GParamSpec       *pspec);
static void     realize        (GtkWidget        *widget);

static void
gpds_event_feedback_class_init (GpdsEventFeedbackClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gobject_class->constructor  = constructor;
    gobject_class->dispose      = dispose;
    gobject_class->set_property = set_property;
    gobject_class->get_property = get_property;

    widget_class->realize       = realize;

    g_object_class_install_property
        (gobject_class,
         PROP_PARENT,
         g_param_spec_object("parent",
             "Parent Window",
             "The parent window",
             GTK_TYPE_WIDGET,
             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_type_class_add_private(gobject_class, sizeof(GpdsEventFeedbackPriv));
}

static void
set_animation (GpdsEventFeedback *feedback, GdkPixbufAnimation *animation)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(feedback);
    GdkPixbuf *pixbuf;
    GdkBitmap *mask = NULL;
    GdkWindow *window;

    gtk_image_set_from_animation(GTK_IMAGE(priv->image), animation);

    window = gtk_widget_get_window(GTK_WIDGET(feedback));
    if (!window)
        return;

    pixbuf = gdk_pixbuf_animation_get_static_image(animation);
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, NULL, &mask, 1);
    gdk_window_shape_combine_mask(window, mask, 0, 0);
    gdk_window_clear(window);
    g_object_unref(mask);
}

static void
realize (GtkWidget *widget)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(widget);
    GdkWindow *window;
    GdkPixbuf *pixbuf;
    GdkPixbufAnimation *animation;
    GdkBitmap *mask = NULL;

    GTK_WIDGET_CLASS(gpds_event_feedback_parent_class)->realize(widget);
    window = gtk_widget_get_window(widget);

    animation = gtk_image_get_animation(priv->image);
    pixbuf = gdk_pixbuf_animation_get_static_image(animation);
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, NULL, &mask, 1);
    gdk_window_shape_combine_mask(window, mask, 0, 0);
    g_object_unref(mask);
}

static gboolean
cb_button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(user_data);
    set_animation(GPDS_EVENT_FEEDBACK(user_data), priv->buttons[event->button - 1]);
    gtk_widget_show(GTK_WIDGET(user_data));
    return FALSE;
}

static gboolean
cb_button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    gtk_widget_hide(GTK_WIDGET(user_data));
    return FALSE;
}

static gboolean
cb_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    GtkRequisition size;

    gtk_widget_size_request(GTK_WIDGET(user_data), &size);
    gtk_window_move(GTK_WINDOW(user_data),
                    event->x_root - size.width / 2,
                    event->y_root - size.height / 2);
    return FALSE;
}

static gboolean
finish_scrolling (gpointer data)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(data);

    if (priv->is_finished_scrolling) {
        gtk_widget_hide(GTK_WIDGET(data));
        priv->is_scrolling = FALSE;
        priv->scroll_direction = -1;
        priv->scroll_timeout_id = 0;
        return FALSE;
    }

    priv->is_finished_scrolling = TRUE;

    return TRUE;
}

static gboolean
cb_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(user_data);
    GtkRequisition size;

    priv->is_finished_scrolling = FALSE;
    if (priv->is_scrolling && priv->scroll_direction == event->direction)
        return FALSE;

    priv->is_scrolling = TRUE;

    if (priv->scroll_direction != event->direction) {
        set_animation(GPDS_EVENT_FEEDBACK(user_data), priv->scrollings[event->direction]);
        gtk_widget_size_request(GTK_WIDGET(user_data), &size);
        gtk_window_move(GTK_WINDOW(user_data),
                        event->x_root - size.width / 2,
                        event->y_root - size.height / 2);
        gtk_widget_show(GTK_WIDGET(user_data));
        if (priv->scroll_timeout_id == 0)
            priv->scroll_timeout_id = g_timeout_add(200, finish_scrolling, user_data);
    }
    priv->scroll_direction = event->direction;

    return FALSE;
}

static GObject *
constructor (GType type, guint n_props, GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass;
    GpdsEventFeedbackPriv *priv;

    klass = G_OBJECT_CLASS(gpds_event_feedback_parent_class);
    object = klass->constructor(type, n_props, props);

    priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(object);
    gtk_window_set_transient_for(GTK_WINDOW(object), GTK_WINDOW(priv->parent));
    g_signal_connect(priv->parent, "button-press-event",
                     G_CALLBACK(cb_button_press), object);
    g_signal_connect(priv->parent, "button-release-event",
                     G_CALLBACK(cb_button_release), object);
    g_signal_connect(priv->parent, "motion-notify-event",
                     G_CALLBACK(cb_motion_notify), object);
    g_signal_connect(priv->parent, "scroll-event",
                     G_CALLBACK(cb_scroll), object);

    return object;
}

#define IMAGE_SIZE 64

static GdkPixbuf *
cairo_surface_to_pixbuf (cairo_surface_t *surface)
{
    gint width, height;
    gint x, y;
    int n_channels;
    guchar *src;
    guchar *dest;
    GdkPixbuf *pixbuf;

    width = cairo_image_surface_get_width(surface);
    height = cairo_image_surface_get_height(surface);

    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                            TRUE,
                            8,
                            width, height);
    n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    src = cairo_image_surface_get_data(surface);
    dest = gdk_pixbuf_get_pixels(pixbuf);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            guchar alpha;
            guchar red, green, blue;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
            alpha = src[3];
            red   = src[2];
            green = src[1];
            blue  = src[0];
#else
            alpha = src[0];
            red   = src[3];
            green = src[1];
            blue  = src[2];
#endif
            if (alpha == 0) {
                dest[0] = 0;
                dest[1] = 0;
                dest[2] = 0;
            } else {
                dest[0] = (red   * 0xff / alpha) & 0xff;
                dest[1] = (green * 0xff / alpha) & 0xff;
                dest[2] = (blue  * 0xff / alpha) & 0xff;
            }
            dest[3] = alpha;

            src += n_channels;
            dest += n_channels;
        }
    }

    return pixbuf;
}

#define N_ANIMATION_FRAMES 8

static GdkPixbuf *
create_number_image (guint8 number, guint frame)
{
    cairo_t *cr;
    cairo_surface_t *surface;
    cairo_text_extents_t extents;
    cairo_pattern_t *pattern;
    GdkPixbuf *pixbuf;
    gdouble start;
    gchar *text;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                         IMAGE_SIZE, IMAGE_SIZE);

    cr = cairo_create(surface);
    cairo_save(cr);

    cairo_rectangle(cr, 0., 0., IMAGE_SIZE, IMAGE_SIZE);
    cairo_set_source_rgba(cr, 0., 0., 0., 0);
    cairo_fill(cr);

    cairo_set_font_size(cr, IMAGE_SIZE);
    cairo_select_font_face(cr, "serif",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);

    text = g_strdup_printf("%d", number);
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr,
                  (IMAGE_SIZE - extents.width) / 2,
                  IMAGE_SIZE - (IMAGE_SIZE - extents.height) / 2);
    cairo_text_path(cr, text);
    g_free(text);

    start = cos(G_PI * frame / N_ANIMATION_FRAMES) * (IMAGE_SIZE / 4);
    pattern = cairo_pattern_create_radial(IMAGE_SIZE / 2,
                                          IMAGE_SIZE / 2,
                                          start,
                                          IMAGE_SIZE / 2,
                                          IMAGE_SIZE / 2,
                                          start + IMAGE_SIZE / 4);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);

    cairo_pattern_add_color_stop_rgba(pattern, 0.,
                                      1., 0., 0.,
                                      1.);
    cairo_pattern_add_color_stop_rgba(pattern, 1.,
                                      0., 0., 1.,
                                      1.);

    cairo_set_source(cr, pattern);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(pattern);

    cairo_set_source_rgba(cr, 1., 1., 1., 1);
    cairo_set_line_width(cr, 2);
    cairo_stroke(cr);
    cairo_restore(cr);
    cairo_destroy(cr);

    pixbuf = cairo_surface_to_pixbuf(surface);
    cairo_surface_destroy(surface);

    return pixbuf;
}

static GdkPixbuf *
create_up_arrow_image (guint frame)
{
    cairo_t *cr;
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    GdkPixbuf *pixbuf;
    gdouble start;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                         IMAGE_SIZE, IMAGE_SIZE);


    cr = cairo_create(surface);
    cairo_save(cr);

    cairo_rectangle(cr, 0., 0., IMAGE_SIZE, IMAGE_SIZE);
    cairo_set_source_rgba(cr, 0., 0., 0., 0);
    cairo_fill(cr);

    cairo_move_to(cr, IMAGE_SIZE / 4,     IMAGE_SIZE);
    cairo_line_to(cr, IMAGE_SIZE / 4,     IMAGE_SIZE / 2);
    cairo_line_to(cr, 0,                  IMAGE_SIZE / 2);
    cairo_line_to(cr, IMAGE_SIZE / 2,     0);
    cairo_line_to(cr, IMAGE_SIZE,         IMAGE_SIZE / 2);
    cairo_line_to(cr, IMAGE_SIZE / 4 * 3, IMAGE_SIZE / 2);
    cairo_line_to(cr, IMAGE_SIZE / 4 * 3, IMAGE_SIZE);
    cairo_close_path(cr);

    start = cos(G_PI * frame / N_ANIMATION_FRAMES) * (IMAGE_SIZE / 4);
    pattern = cairo_pattern_create_linear(0., start,
                                          0., start + (IMAGE_SIZE / 4));
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
    cairo_pattern_add_color_stop_rgba(pattern, 0.,
                                      0., 0., 1.,
                                      0.);
    cairo_pattern_add_color_stop_rgba(pattern, 1.,
                                      0., 0., 1.,
                                      1.);

    cairo_set_source(cr, pattern);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(pattern);

    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgba(cr, 1., 1., 1., 1.);
    cairo_set_line_width(cr, 4);
    cairo_stroke(cr);
    cairo_restore(cr);
    cairo_destroy(cr);

    pixbuf = cairo_surface_to_pixbuf(surface);
    cairo_surface_destroy(surface);

    return pixbuf;
}

static GdkPixbufAnimation *
create_button_images (guint button_number)
{
    GdkPixbufSimpleAnim*images;
    gint i;

    images = gdk_pixbuf_simple_anim_new(IMAGE_SIZE, IMAGE_SIZE,
                                        N_ANIMATION_FRAMES / 2);
    gdk_pixbuf_simple_anim_set_loop(images, TRUE);

    for (i = 0; i < N_ANIMATION_FRAMES; i++) {
        GdkPixbuf *pixbuf;
        pixbuf = create_number_image(button_number, i);
        gdk_pixbuf_simple_anim_add_frame(images, pixbuf);
        g_object_unref(pixbuf);
    }

    return GDK_PIXBUF_ANIMATION(images);
}

static GdkPixbufAnimation *
create_arrow_images (GdkScrollDirection direction)
{
    GdkPixbufSimpleAnim*images;
    gint i;

    images = gdk_pixbuf_simple_anim_new(IMAGE_SIZE, IMAGE_SIZE,
                                        N_ANIMATION_FRAMES / 2);
    gdk_pixbuf_simple_anim_set_loop(images, TRUE);

    for (i = 0; i < N_ANIMATION_FRAMES; i++) {
        GdkPixbuf *pixbuf;
        GdkPixbuf *rotate;
        GdkPixbufRotation angle;

        pixbuf = create_up_arrow_image(i);
        switch (direction) {
        case GDK_SCROLL_RIGHT:
            angle = GDK_PIXBUF_ROTATE_CLOCKWISE;
            break;
        case GDK_SCROLL_LEFT:
            angle = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
            break;
        case GDK_SCROLL_DOWN:
            angle = GDK_PIXBUF_ROTATE_UPSIDEDOWN;
            break;
        case GDK_SCROLL_UP:
        default:
            angle = GDK_PIXBUF_ROTATE_NONE;
            break;
        }

        rotate = gdk_pixbuf_rotate_simple(pixbuf, angle);
        gdk_pixbuf_simple_anim_add_frame(images, rotate);
        g_object_unref(pixbuf);
        g_object_unref(rotate);
    }

    return GDK_PIXBUF_ANIMATION(images);
}

static void
gpds_event_feedback_init (GpdsEventFeedback *feedback)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(feedback);
    gint i;

    priv->parent = NULL;
    priv->is_scrolling = FALSE;
    priv->scroll_direction = -1;
    priv->is_finished_scrolling = FALSE;
    priv->scroll_timeout_id = 0;

    for (i = 0; i < 5; i++)
        priv->buttons[i] = create_button_images(i + 1);

    for (i = 0; i < 4; i++)
        priv->scrollings[i] = create_arrow_images(i);

    priv->image = GTK_IMAGE(gtk_image_new());
    gtk_container_add(GTK_CONTAINER(feedback), GTK_WIDGET(priv->image));
    gtk_widget_show(GTK_WIDGET(priv->image));
}

static void
dispose (GObject *object)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(object);
    gint i;

    if (priv->parent) {
        g_object_unref(priv->parent);
        priv->parent = NULL;
    }

    if (priv->scroll_timeout_id) {
        g_source_remove(priv->scroll_timeout_id);
        priv->scroll_timeout_id = 0;
    }

    for (i = 0; i < 5; i++) {
        if (priv->buttons[i]) {
            g_object_unref(priv->buttons[i]);
            priv->buttons[i] = NULL;
        }
    }

    for (i = 0; i < 4; i++) {
        if (priv->scrollings[i]) {
            g_object_unref(priv->scrollings[i]);
            priv->scrollings[i] = NULL;
        }
    }

    if (G_OBJECT_CLASS(gpds_event_feedback_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_event_feedback_parent_class)->dispose(object);
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_PARENT:
        priv->parent = g_value_dup_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
    GpdsEventFeedbackPriv *priv = GPDS_EVENT_FEEDBACK_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_PARENT:
        g_value_set_object(value, priv->parent);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

GtkWidget *
gpds_event_feedback_new (GtkWindow *parent)
{
    return GTK_WIDGET(g_object_new(GPDS_TYPE_EVENT_FEEDBACK,
                                   "type", GTK_WINDOW_POPUP,
                                   "type-hint", GDK_WINDOW_TYPE_HINT_NOTIFICATION,
                                   "destroy-with-parent", TRUE,
                                   "parent", parent,
                                   NULL));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
