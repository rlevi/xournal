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
 * SECTION:he-color-button
 * @short_description: A widget to open HeColorDialog
 * @see_also: #HeColorSelector, #HeColorDialog
 *
 * #HeColorButton is a widget to open a HeColorDialog.
 * The selected color is shown in the button.
 * The selected color is a property of the button.
 * The property name is "color" and its type is GtkColor.
 *
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <langinfo.h>

#include "he-color-button.h"
#include "he-simple-color-dialog.h"

#define HE_COLOR_BUTTON_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_COLOR_BUTTON, HeColorButtonPrivate))

#define _(String) dgettext("hildon-libs", String)

struct _HeColorButtonPrivate
{
	GtkWidget *dialog;
	gboolean popped;

	GdkColor color;
	GdkGC *gc;
};

enum
{
    PROP_0,
    PROP_COLOR,
    PROP_POPUP_SHOWN
};

static GObject * he_color_button_constructor (GType                  type,
                                                   guint                  n_construct_properties,
                                                   GObjectConstructParam *construct_properties);

static void he_color_button_clicked (GtkButton *button);
static void he_color_button_class_init (HeColorButtonClass * class);
static void he_color_button_init (HeColorButton *cb);

static gpointer                                 parent_class = NULL;

/* private functions */
/**
  * he_color_button_get_type:
  *
  * Initializes and returns the type of a he color button.
  *
  * Returns: GType of #HeColorButton.
  */
 GType G_GNUC_CONST
 he_color_button_get_type                    (void)
 {
     static GType color_button_type = 0;
	 color_button_type = g_type_from_name ("HeColorButton");
 
     if (! color_button_type)
     {
         static const GTypeInfo color_button_info =
         {
             sizeof (HeColorButtonClass),
             NULL,           /* base_init */
             NULL,           /* base_finalize */
             (GClassInitFunc) he_color_button_class_init,
             NULL,           /* class_finalize */
             NULL,           /* class_data */
             sizeof (HeColorButton),
             0,              /* n_preallocs */
             (GInstanceInitFunc) he_color_button_init,
         };
 
         color_button_type = g_type_register_static (GTK_TYPE_BUTTON, "HeColorButton",
                 &color_button_info, 0);
     }
 
     return color_button_type;
 }

static void
he_color_button_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	//g_warning ("he_color_button_set_property");
  HeColorButton *cb = HE_COLOR_BUTTON (object);
  HeColorButtonPrivate *priv = HE_COLOR_BUTTON_GET_PRIVATE (cb);

  switch (prop_id)
  {
  case PROP_COLOR:
    priv->color = *(GdkColor *) g_value_get_boxed (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
he_color_button_get_property (GObject      *object,
                                   guint         prop_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
	//g_warning ("he_color_button_get_property");
  HeColorButton *cb = HE_COLOR_BUTTON (object);
  HeColorButtonPrivate *priv = HE_COLOR_BUTTON_GET_PRIVATE (cb);

  switch (prop_id)
  {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;
    case PROP_POPUP_SHOWN:
      g_value_set_boolean (value, priv->popped);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
he_color_button_class_init (HeColorButtonClass * class)
{
  //g_warning ("he_color_button_class_init");
  GObjectClass *gobject_class;
  GtkButtonClass *button_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass *) class;
  button_class = (GtkButtonClass *) class;
  widget_class = (GtkWidgetClass *) class;
  
  parent_class = g_type_class_peek_parent (class);

  gobject_class->get_property = he_color_button_get_property;
  gobject_class->set_property = he_color_button_set_property;
  button_class->clicked = he_color_button_clicked;

  g_object_class_install_property (
    gobject_class,
    PROP_COLOR,
    g_param_spec_boxed (
      "color",
      "Current Color",
      "The selected color",
      GDK_TYPE_COLOR,
	  G_PARAM_READWRITE));

  g_object_class_install_property (
    gobject_class,
    PROP_POPUP_SHOWN,
    g_param_spec_boolean (
      "popup-shown",
      "IsPopped",
      "If the color dialog is popped up",
      FALSE,
	  G_PARAM_READABLE));

  g_type_class_add_private (button_class, sizeof (HeColorButtonPrivate));
}

static void
he_color_button_init (HeColorButton *cb)
{
  g_warning ("he_color_button_init");
  cb->priv = HE_COLOR_BUTTON_GET_PRIVATE (cb);
  cb->priv->dialog=NULL;
  
  GtkWidget *event = gtk_event_box_new ();
  //gtk_widget_set_size_request (event, 20,20);
  gtk_container_add (GTK_CONTAINER (cb), event);
  gtk_widget_show_all (GTK_WIDGET (cb));

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (cb), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (cb), FALSE);
}

static void
he_color_button_finalize (GObject * object)
{
  //g_warning ("he_color_button_finalize");
  HeColorButtonPrivate *priv = HE_COLOR_BUTTON_GET_PRIVATE (object);

  if (priv->dialog)
  {
	  gtk_widget_destroy (priv->dialog);
	  priv->dialog = NULL;
  }
  
  if (G_OBJECT_CLASS (parent_class)->finalize)
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */
static void
he_color_button_clicked (GtkButton *button)
{
	//g_warning ("he_color_button_clicked");
	HeColorButton *cb = HE_COLOR_BUTTON (button);
	HeSimpleColorDialog *c_dialog;
	c_dialog = (HeSimpleColorDialog *) cb->priv->dialog;
	if (! c_dialog)
	{
		GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (cb));
		cb->priv->dialog = he_simple_color_dialog_new ();
		he_simple_color_dialog_set_color (HE_SIMPLE_COLOR_DIALOG (cb->priv->dialog), &cb->priv->color);
		c_dialog = HE_SIMPLE_COLOR_DIALOG (cb->priv->dialog);
		if (parent)
			gtk_window_set_transient_for (GTK_WINDOW (c_dialog), GTK_WINDOW (parent));
	}
	he_simple_color_dialog_set_color (c_dialog, &cb->priv->color);
	
	cb->priv->popped = TRUE;

	if (gtk_dialog_run (GTK_DIALOG (c_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GdkColor *color = he_simple_color_dialog_get_color (c_dialog);
		he_color_button_set_color (cb, color);
	}
	gtk_widget_hide (GTK_WIDGET (cb->priv->dialog));
	cb->priv->popped = FALSE;
}


/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * he_color_button_new:
 *
 * Creates a new #HeColorButton
 *
 * Returns: a new #HeColorButton
 *
 * Since: 2.2
 **/
GtkWidget *
he_color_button_new ()
{
  //g_warning ("he_color_button_new");
  return g_object_new (HE_TYPE_COLOR_BUTTON, NULL);
}

/**
 * he_color_button_new_with_color:
 * @dcolor: the color to initialize to
 *
 * Creates a new #HeColorButton with a specific color.
 *
 * Returns: a new #HeColorButton
 *
 * Since: 2.2
 **/
GtkWidget *
he_color_button_new_with_color (const GdkColor *color)
{
  //g_warning ("he_color_button_new_with_color");
  GtkWidget *cb = g_object_new (HE_TYPE_COLOR_BUTTON, "color", color, NULL);
  he_color_button_set_color (HE_COLOR_BUTTON (cb), color);
  return cb;
}

/**
 * he_color_button_set_color:
 * @cb: the #HeColorButton
 * @color: the color to set
 * 
 * Sets the current color on the #HeColorButton widget
 * 
 * Since: 2.2
 * 
 **/
void
he_color_button_set_color (HeColorButton *cb,
                           const GdkColor *color)
{
	//g_warning ("he_color_button_set_color");
	g_return_if_fail (HE_IS_COLOR_BUTTON (cb));
	
	g_object_set (G_OBJECT (cb), "color", color, NULL);
	cb->priv->color.red = color->red;
	cb->priv->color.green = color->green;
	cb->priv->color.blue = color->blue;
	cb->priv->color.pixel = color->pixel;
	gtk_widget_modify_bg (gtk_bin_get_child (GTK_BIN (cb)), GTK_STATE_NORMAL, color);
	gtk_widget_modify_bg (gtk_bin_get_child (GTK_BIN (cb)), GTK_STATE_PRELIGHT, color);
}

/**
 * he_color_button_get_popup_down:
 * @cb: the #HeColorButton
 *
 * This function checks if the color button has the color
 * dialog currently popped-up
 *
 * Since: 2.2
 *
 * Returns: %TRUE if the dialog is popped-up (visible to user)
 **/
gboolean
he_fcolor_button_get_popup_shown (HeColorButton * cb)
{
	//g_warning ("he_color_button_get_popup_shown");
  g_return_val_if_fail (HE_IS_COLOR_BUTTON (cb), FALSE);
  
  return cb->priv->popped;
}

/**
 * he_color_button_popdown:
 * @cb: the #HeColorButton
 *
 * If the color dialog is currently popped-up (visible)
 * it will be popped-down (hidden).
 *
 * Since: 2.2
 **/
void
he_color_button_popdown (HeColorButton * cb)
{
	//g_warning ("he_color_button_popdown");
	g_return_if_fail (HE_IS_COLOR_BUTTON (cb));
	
	if (cb->priv->popped && cb->priv->dialog)
	{
		gtk_dialog_response (GTK_DIALOG (cb->priv->dialog), GTK_RESPONSE_CANCEL);
	}
}

/**
 * he_color_button_get_color:
 * @cb: the #HeColorButton
 * @color: a color #GdkColor to be filled with the current color
 *
 * Since: 2.2
 **/
void
he_color_button_get_color (HeColorButton *cb, GdkColor *color)
{
  //g_warning ("he_color_button_get_color");
  g_return_if_fail (HE_IS_COLOR_BUTTON (cb));
  g_return_if_fail (color != NULL);
  
  color->red = cb->priv->color.red;
  color->green = cb->priv->color.green;
  color->blue = cb->priv->color.blue;
  color->pixel = cb->priv->color.pixel;

}
