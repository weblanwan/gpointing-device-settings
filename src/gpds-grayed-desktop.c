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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gpds-grayed-desktop.h"
#include "gpds-utils.h"

enum
{
    PROP_0,
    PROP_MAIN_WINDOW
};

typedef struct _GpdsGrayedDesktopPriv GpdsGrayedDesktopPriv;
struct _GpdsGrayedDesktopPriv
{
    GtkWidget *main_window;
    GdkPixbuf *background;
};

#define GPDS_GRAYED_DESKTOP_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GPDS_TYPE_GRAYED_DESKTOP, GpdsGrayedDesktopPriv))

G_DEFINE_TYPE(GpdsGrayedDesktop, gpds_grayed_desktop, GTK_TYPE_WINDOW)

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
static gboolean expose         (GtkWidget        *widget,
					            GdkEventExpose   *event);

static void
gpds_grayed_desktop_class_init (GpdsGrayedDesktopClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gobject_class->constructor  = constructor;
    gobject_class->dispose      = dispose;
    gobject_class->set_property = set_property;
    gobject_class->get_property = get_property;

    widget_class->expose_event = expose;

    g_object_class_install_property
        (gobject_class,
         PROP_MAIN_WINDOW,
         g_param_spec_object("main-window",
             "Main Window",
             "The main window",
             GTK_TYPE_WIDGET,
             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_type_class_add_private(gobject_class, sizeof(GpdsGrayedDesktopPriv));
}

static GdkPixbuf *
create_grayscaled_background (void)
{
    GdkWindow *root;
    GdkPixbuf *pixbuf;
    gint width, height;
    guchar *pixels;
    int rowstride, n_channels;
    int x, y;

    root = gdk_get_default_root_window();
    gdk_drawable_get_size(root, &width, &height);
    pixbuf = gdk_pixbuf_get_from_drawable(NULL,
                                          root,
                                          NULL,
                                          0, 0,
                                          0, 0,
                                          width, height);

    pixels = gdk_pixbuf_get_pixels(pixbuf);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width * n_channels; x += n_channels) {
            guchar grayscale;
            guchar *p;

            p = pixels + y * rowstride + x;
            grayscale = (p[0] * 11 + p[1] * 16 + p[2] * 5) / 32;
            p[0] = grayscale;
            p[1] = grayscale;
            p[2] = grayscale;
            p[3] = 1;
        }
    }

    return pixbuf;
}

static void
cb_main_window_destroyed (GtkWindow *main_window, gpointer user_data)
{
    gtk_widget_destroy(GTK_WIDGET(user_data));
}

static GObject *
constructor (GType type, guint n_props, GObjectConstructParam *props)
{
    GObject *object;
    GObjectClass *klass;
    GpdsGrayedDesktopPriv *priv;
    gint x, y;

    klass = G_OBJECT_CLASS(gpds_grayed_desktop_parent_class);
    object = klass->constructor(type, n_props, props);

    priv = GPDS_GRAYED_DESKTOP_GET_PRIVATE(object);
    gtk_window_get_position(GTK_WINDOW(priv->main_window), &x, &y);
    gtk_widget_hide(GTK_WIDGET(priv->main_window));
    priv->background = create_grayscaled_background();
    gtk_widget_show_now(GTK_WIDGET(priv->main_window));

    gtk_window_set_transient_for(GTK_WINDOW(priv->main_window), GTK_WINDOW(object));
    gtk_window_move(GTK_WINDOW(priv->main_window), x, y);
    g_signal_connect(priv->main_window, "destroy",
                     G_CALLBACK(cb_main_window_destroyed), object);

    return object;
}

static gboolean
expose (GtkWidget *widget, GdkEventExpose *event)
{
    GpdsGrayedDesktopPriv *priv = GPDS_GRAYED_DESKTOP_GET_PRIVATE(widget);
    GdkWindow *window;

    window = gtk_widget_get_window(widget);

    gdk_draw_pixbuf(window,
                    NULL,
                    priv->background,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height,
                    GDK_RGB_DITHER_NONE,
                    0, 0);

    return FALSE;
}

static void
gpds_grayed_desktop_init (GpdsGrayedDesktop *window)
{
    GpdsGrayedDesktopPriv *priv = GPDS_GRAYED_DESKTOP_GET_PRIVATE(window);

    priv->main_window = NULL;
    priv->background = NULL;

    gtk_window_fullscreen(GTK_WINDOW(window));
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
}

static void
dispose (GObject *object)
{
    GpdsGrayedDesktopPriv *priv = GPDS_GRAYED_DESKTOP_GET_PRIVATE(object);

    if (priv->main_window) {
        g_object_unref(priv->main_window);
        priv->main_window = NULL;
    }

    if (priv->background) {
        g_object_unref(priv->background);
        priv->background = NULL;
    }

    if (G_OBJECT_CLASS(gpds_grayed_desktop_parent_class)->dispose)
        G_OBJECT_CLASS(gpds_grayed_desktop_parent_class)->dispose(object);
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
    GpdsGrayedDesktopPriv *priv = GPDS_GRAYED_DESKTOP_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_MAIN_WINDOW:
        priv->main_window = g_value_dup_object(value);
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
    GpdsGrayedDesktopPriv *priv = GPDS_GRAYED_DESKTOP_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_MAIN_WINDOW:
        g_value_set_object(value, priv->main_window);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

GtkWidget *
gpds_grayed_desktop_new (GtkWindow *main_window)
{
    return GTK_WIDGET(g_object_new(GPDS_TYPE_GRAYED_DESKTOP,
                                   "type", GTK_WINDOW_TOPLEVEL,
                                   "type-hint", GDK_WINDOW_TYPE_HINT_NORMAL,
                                   "main-window", main_window,
                                   NULL));
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
