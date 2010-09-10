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
 * SECTION:he-color-size-dialog
 * @short_description: A dialog to allow the user to select colors
 * @see_also: #HeColorSizeSelector, #HeColorButton
 *
 * #HeColorSizeDialog enables the user to select an arbitrary
 * color from a HSV colorspace.
 *
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <langinfo.h>

#include "he-color-size-dialog.h"
#include "he-color-size-selector.h"

#define HE_COLOR_SIZE_DIALOG_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_COLOR_SIZE_DIALOG, HeColorSizeDialogPrivate))

#define _(String) dgettext("hildon-libs", String)

struct _HeColorSizeDialogPrivate
{
	HeColorSizeSelector *selector;
};

static GObject * he_color_size_dialog_constructor (GType                  type,
                                                   guint                  n_construct_properties,
                                                   GObjectConstructParam *construct_properties);

void he_color_size_dialog_response (GtkDialog *dialog, gint response_id, HeColorSizeDialog *cd);
static void he_color_size_dialog_class_init (HeColorSizeDialogClass * class);
static void he_color_size_dialog_init (HeColorSizeDialog * cd);

static gpointer                                 parent_class = NULL;

GType G_GNUC_CONST
he_color_size_dialog_get_type            (void)
{
    static GType dialog_type = 0;
	dialog_type = g_type_from_name ("HeColorSizeDialog");

    if (!dialog_type) {
        static const GTypeInfo dialog_info =
        {
            sizeof (HeColorSizeDialogClass),
            NULL,
            NULL,
            (GClassInitFunc) he_color_size_dialog_class_init,
            NULL,
            NULL,
            sizeof (HeColorSizeDialog),
            0,
            (GInstanceInitFunc) he_color_size_dialog_init,
            NULL
        };

        dialog_type = g_type_register_static (GTK_TYPE_DIALOG, 
                "HeColorSizeDialog", &dialog_info, 0);
    }

    return dialog_type;
}

/* private functions */
static void
he_color_size_dialog_class_init (HeColorSizeDialogClass * class)
{
  //g_warning ("he_color_size_dialog_class_init");
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass *) class;
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  
  parent_class = g_type_class_peek_parent (class);

  g_type_class_add_private (object_class, sizeof (HeColorSizeDialogPrivate));
}

static void
he_color_size_dialog_init (HeColorSizeDialog *cd)
{
  //g_warning ("he_color_size_dialog_init");
  cd->priv = HE_COLOR_SIZE_DIALOG_GET_PRIVATE (cd);
  
  gtk_dialog_set_has_separator (GTK_DIALOG (cd), FALSE);
  gtk_window_set_title (GTK_WINDOW (cd), "Select color");
  
  GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (cd));
  cd->priv->selector = HE_COLOR_SIZE_SELECTOR (he_color_size_selector_new ());
  gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET (cd->priv->selector));
  gtk_dialog_add_button (GTK_DIALOG (cd), _("wdgt_bd_done"), GTK_RESPONSE_ACCEPT);
  g_signal_connect (G_OBJECT (cd), "response", G_CALLBACK (he_color_size_dialog_response), cd);
  gtk_widget_show_all (GTK_WIDGET (cd));

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (cd), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (cd), FALSE);
}

static void
he_color_size_dialog_finalize (GObject * object)
{
  //g_warning ("he_color_size_dialog_finalize");
  HeColorSizeDialogPrivate *priv = HE_COLOR_SIZE_DIALOG_GET_PRIVATE (object);

  if (G_OBJECT_CLASS (parent_class)->finalize)
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */
void
he_color_size_dialog_response (GtkDialog *dialog, gint response_id, HeColorSizeDialog *cd)
{
	//g_warning ("he_color_size_dialog_response");
	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		GdkColor color;
		he_color_size_selector_get_color (cd->priv->selector, &color);
		he_color_size_dialog_set_color (cd, &color);
	}
}


/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * he_color_size_dialog_new:
 *
 * Creates a new #HeColorSizeDialog
 *
 * Returns: a new #HeColorSizeDialog
 *
 * Since: 2.2
 **/
GtkWidget *
he_color_size_dialog_new ()
{
  //g_warning ("he_color_size_dialog_new");
  return g_object_new (HE_TYPE_COLOR_SIZE_DIALOG, NULL);
}

/**
 * he_color_size_dialog_new_with_color:
 * @dcolor: the color to initialize to
 *
 * Creates a new #HeColorSizeDialog with a specific color.
 *
 * Returns: a new #HeColorSizeDialog
 *
 * Since: 2.2
 **/
GtkWidget *
he_color_size_dialog_new_with_color (GdkColor *color)
{
  //g_warning ("he_color_size_dialog_new_with_color");
  GtkWidget *cd = g_object_new (HE_TYPE_COLOR_SIZE_DIALOG, NULL);
  he_color_size_dialog_set_color (HE_COLOR_SIZE_DIALOG (cd), color);
  return cd;
}

/**
 * he_color_size_dialog_set_color:
 * @cb: the #HeColorSizeDialog
 * @color: the color to set
 * 
 * Sets the current color on the #HeColorSizeDialog widget
 * 
 * Since: 2.2
 * 
 **/
void
he_color_size_dialog_set_color (HeColorSizeDialog *cd,
                                          GdkColor *color)
{
	//g_warning ("he_color_size_dialog_set_color");
	g_return_if_fail (HE_IS_COLOR_SIZE_DIALOG (cd));
	
	he_color_size_selector_set_color (cd->priv->selector, color);
}

/**
 * he_color_size_dialog_get_color:
 * @cb: the #HeColorSizeDialog
 * @color: a color #GdkColor to be filled with the current color
 *
 * Since: 2.2
 **/
void
he_color_size_dialog_get_color (HeColorSizeDialog *cd, GdkColor *color)
{
  //g_warning ("he_color_size_dialog_get_color");
  g_return_if_fail (HE_IS_COLOR_SIZE_DIALOG (cd));
  g_return_if_fail (color != NULL);
  
  he_color_size_selector_get_color (cd->priv->selector, color);

}
