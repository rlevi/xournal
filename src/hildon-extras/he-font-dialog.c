/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2009 Andrew Olmsted
 * Copyright (C) 2005, 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version. or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * SECTION:he-font-dialog
 * @short_description: A dialog to allow the user to select fonts
 * @see_also: #HeFontSelector, #HeFontButton
 *
 * #HeFontDialog enables the user to select a font and font attributes
 * from a list of available fonts
 *
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <langinfo.h>

#include "he-font-selector.h"
#include "he-font-dialog.h"

#define HE_FONT_DIALOG_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_FONT_DIALOG, HeFontDialogPrivate))

#define _(String) dgettext("hildon-libs", String)

struct _HeFontDialogPrivate
{
	HeFontSelector *selector;
};

static GObject * he_font_dialog_constructor (GType                  type,
                                                   guint                  n_construct_properties,
                                                   GObjectConstructParam *construct_properties);
static void he_font_dialog_finalize (GObject * object);

void he_font_dialog_response (GtkDialog *dialog, gint response_id, HeFontDialog *fd);
static void he_font_dialog_class_init (HeFontDialogClass * class);
static void he_font_dialog_init (HeFontDialog * fd);
static void he_font_dialog_show (GtkWidget *widget);
static void he_font_dialog_destroy (GtkObject *object);

static gpointer                                 parent_class = NULL;

GType G_GNUC_CONST
he_font_dialog_get_type            (void)
{
    static GType dialog_type = 0;
	dialog_type = g_type_from_name ("HeFontDialog");

    if (!dialog_type) {
        static const GTypeInfo dialog_info =
        {
            sizeof (HeFontDialogClass),
            NULL,
            NULL,
            (GClassInitFunc) he_font_dialog_class_init,
            NULL,
            NULL,
            sizeof (HeFontDialog),
            0,
            (GInstanceInitFunc) he_font_dialog_init,
            NULL
        };

        dialog_type = g_type_register_static (GTK_TYPE_DIALOG, 
                "HeFontDialog", &dialog_info, 0);
    }

    return dialog_type;
}

static void
he_font_dialog_class_init (HeFontDialogClass * class)
{
  //g_warning ("he_font_dialog_class_init");
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass *) class;
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  
  parent_class = g_type_class_peek_parent (class);

  g_type_class_add_private (object_class, sizeof (HeFontDialogPrivate));
}

static void
he_font_dialog_show (GtkWidget *widget)
{
	HeFontDialogPrivate *priv = HE_FONT_DIALOG_GET_PRIVATE (widget);
	gtk_widget_show (GTK_WIDGET (priv->selector));
	
	GTK_WIDGET_CLASS (parent_class)->show (widget);
}

static void
he_font_dialog_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
he_font_dialog_init (HeFontDialog *fd)
{
  //g_warning ("he_font_dialog_init");
  fd->priv = HE_FONT_DIALOG_GET_PRIVATE (fd);
  
  gtk_dialog_set_has_separator (GTK_DIALOG (fd), FALSE);
  gtk_window_set_title (GTK_WINDOW (fd), "Select font");
  
  GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (fd));
  fd->priv->selector = HE_FONT_SELECTOR (he_font_selector_new ());
  gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET (fd->priv->selector));
  gtk_dialog_add_button (GTK_DIALOG (fd), _("wdgt_bd_done"), GTK_RESPONSE_ACCEPT);
  g_signal_connect (G_OBJECT (fd), "response", G_CALLBACK (he_font_dialog_response), fd);
  gtk_widget_show_all (GTK_WIDGET (fd));
  gtk_widget_set_size_request (GTK_WIDGET (fd), -1, 350);

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (fd), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (fd), FALSE);
}

static void
he_font_dialog_finalize (GObject * object)
{
  //g_warning ("he_font_dialog_finalize");
  HeFontDialogPrivate *priv = HE_FONT_DIALOG_GET_PRIVATE (object);

  if (G_OBJECT_CLASS (parent_class)->finalize)
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */
void
he_font_dialog_response (GtkDialog *dialog, gint response_id, HeFontDialog *fd)
{
	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		gchar *font = he_font_selector_get_font (fd->priv->selector);
		he_font_dialog_set_font (fd, font);
		g_free (font);
	}
}


/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * he_font_dialog_new:
 *
 * Creates a new #HeFontDialog
 *
 * Returns: a new #HeFontDialog
 *
 * Since: 2.2
 **/
GtkWidget *
he_font_dialog_new ()
{
  //g_warning ("he_font_dialog_new");
  return g_object_new (HE_TYPE_FONT_DIALOG, NULL);
}

/**
 * he_font_dialog_new_with_font:
 * @font: the font to initialize to
 *
 * Creates a new #HeFontDialog with a specific font.
 *
 * Returns: a new #HeFontDialog
 *
 * Since: 2.2
 **/
GtkWidget *
he_font_dialog_new_with_font (gchar *font_string)
{
  //g_warning ("he_font_dialog_new_with_font");
  GtkWidget *fd = g_object_new (HE_TYPE_FONT_DIALOG, NULL);
  he_font_dialog_set_font (HE_FONT_DIALOG (fd), font_string);
  return fd;
}

/**
 * he_font_dialog_set_font:
 * @cb: the #HeFontDialog
 * @font: the font to set
 * 
 * Sets the current font on the #HeFontDialog widget
 * 
 * Since: 2.2
 * 
 **/
void
he_font_dialog_set_font (HeFontDialog *fd,
                         gchar *font_string)
{
	//g_warning ("he_font_dialog_set_font");
	g_return_if_fail (HE_IS_FONT_DIALOG (fd));
	
	he_font_selector_set_font (fd->priv->selector, font_string);
}

/**
 * he_font_dialog_get_font:
 * @cb: the #HeFontDialog
 * 
 * @returns: a gchar* string representing the current font
 *
 * Since: 2.2
 **/
gchar*
he_font_dialog_get_font (HeFontDialog *fd)
{
  //g_warning ("he_font_dialog_get_font");
  g_return_if_fail (HE_IS_FONT_DIALOG (fd));
  
  return he_font_selector_get_font (fd->priv->selector);

}
