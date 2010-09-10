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
 * SECTION:he-color-selector
 * @short_description: A widget to select a color.
 *
 * #HeColorSelector is a color widget with multiple sliders. Users
 * can choose a color by selecting values on the red, green and blue
 * sliders.
 *
 * The current red can be altered with
 * he_color_selector_select_red().  The green can be selected
 * with he_color_selection_select_green().  The blue can be selected
 * with he_color_selection_select_blue().
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <langinfo.h>

#include <hildon/hildon-entry.h>
#include <hildon/hildon-caption.h>

#include "he-color-selector.h"

#define HE_COLOR_SELECTOR_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_COLOR_SELECTOR, HeColorSelectorPrivate))

#define _(String) dgettext("hildon-libs", String)

struct _HeColorSelectorPrivate
{
  GtkWidget *redSlider;
  GtkWidget *greenSlider;
  GtkWidget *blueSlider;
  
  GtkWidget *colorName;
  
  GtkWidget *preview;
  
  gboolean updating;
};

/* private functions */
static void he_color_selector_change_slider_value (GtkRange *range, HeColorSelector *selector);
static void he_color_selector_change_color_name (GtkEditable *editable, HeColorSelector *selector);
static void he_color_selector_update_preview_widget (HeColorSelector *selector);
static void he_color_selector_class_init (HeColorSelectorClass * class);
static void he_color_selector_init (HeColorSelector * selector);

static gpointer                                 parent_class = NULL;

/**
  * he_color_selector_get_type:
  *
  * Initializes and returns the type of a he color selector.
  *
  * Returns: GType of #HeColorSelector.
  */
 GType G_GNUC_CONST
 he_color_selector_get_type                    (void)
 {
     static GType color_selector_type = 0;
	 color_selector_type = g_type_from_name ("HeColorSelector");
 
     if (! color_selector_type)
     {
         static const GTypeInfo color_selector_info =
         {
             sizeof (HeColorSelectorClass),
             NULL,           /* base_init */
             NULL,           /* base_finalize */
             (GClassInitFunc) he_color_selector_class_init,
             NULL,           /* class_finalize */
             NULL,           /* class_data */
             sizeof (HeColorSelector),
             0,              /* n_preallocs */
             (GInstanceInitFunc) he_color_selector_init,
         };
 
         color_selector_type = g_type_register_static (GTK_TYPE_VBOX, "HeColorSelector",
                 &color_selector_info, 0);
     }
 
     return color_selector_type;
 }

static void
he_color_selector_class_init (HeColorSelectorClass * class)
{
  //g_warning ("he_color_selector_class_init");
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  gobject_class = (GObjectClass *) class;
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  container_class = (GtkContainerClass *) class;
  
  parent_class = g_type_class_peek_parent (class);

  g_type_class_add_private (object_class, sizeof (HeColorSelectorPrivate));
}

static void
he_color_selector_init (HeColorSelector * selector)
{
  //g_warning ("he_color_selector_init");
  selector->priv = HE_COLOR_SELECTOR_GET_PRIVATE (selector);

  selector->priv->redSlider = hildon_gtk_hscale_new ();
  selector->priv->greenSlider = hildon_gtk_hscale_new ();
  selector->priv->blueSlider = hildon_gtk_hscale_new ();

  selector->priv->preview = gtk_event_box_new ();
  selector->priv->colorName = hildon_entry_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);

  GtkSizeGroup *group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  GtkWidget *redCaption = hildon_caption_new (group, "Red", selector->priv->redSlider, NULL, HILDON_CAPTION_MANDATORY);
  hildon_caption_set_separator (HILDON_CAPTION (redCaption), "");
  GtkWidget *greenCaption = hildon_caption_new (group, "Green", selector->priv->greenSlider, NULL, HILDON_CAPTION_MANDATORY);
  hildon_caption_set_separator (HILDON_CAPTION (greenCaption), "");
  GtkWidget *blueCaption = hildon_caption_new (group, "Blue", selector->priv->blueSlider, NULL, HILDON_CAPTION_MANDATORY);
  hildon_caption_set_separator (HILDON_CAPTION (blueCaption), "");
  GtkWidget *nameCaption = hildon_caption_new (group, "Name", selector->priv->colorName, NULL, HILDON_CAPTION_MANDATORY);
  hildon_caption_set_separator (HILDON_CAPTION (nameCaption), "");

  g_signal_connect (selector->priv->redSlider, "value-changed", G_CALLBACK (he_color_selector_change_slider_value), selector);
  g_signal_connect (selector->priv->greenSlider, "value-changed", G_CALLBACK (he_color_selector_change_slider_value), selector);
  g_signal_connect (selector->priv->blueSlider, "value-changed", G_CALLBACK (he_color_selector_change_slider_value), selector);
  
  g_signal_connect (selector->priv->colorName, "changed", G_CALLBACK (he_color_selector_change_color_name), selector);

  gtk_container_add (GTK_CONTAINER (vbox), redCaption);
  gtk_container_add (GTK_CONTAINER (vbox), greenCaption);
  gtk_container_add (GTK_CONTAINER (vbox), blueCaption);
  gtk_container_add (GTK_CONTAINER (vbox), nameCaption);
  
  gtk_container_add (GTK_CONTAINER (hbox), vbox);
  gtk_container_add (GTK_CONTAINER (hbox), selector->priv->preview);
  gtk_container_add (GTK_CONTAINER (selector), hbox);

  gtk_widget_set_size_request (selector->priv->preview, 128, 128);

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (selector), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (selector), FALSE);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */

static void
he_color_selector_change_slider_value (GtkRange *range, HeColorSelector *selector)
{
	//g_warning ("he_color_selector_change_slider_value");
	he_color_selector_update_preview_widget (selector);
}

static void
he_color_selector_change_color_name (GtkEditable *editable, HeColorSelector *selector)
{
	//g_warning ("he_color_selector_change_color_name");
	GdkColor color;
	if (!selector->priv->updating)
	{
		if (gdk_color_parse (hildon_entry_get_text (HILDON_ENTRY (editable)), &color))
		{
			selector->priv->updating = TRUE;
			he_color_selector_set_color (selector, &color);
			selector->priv->updating = FALSE;
		}
	}
}

static void
he_color_selector_update_preview_widget (HeColorSelector *selector)
{
	//g_warning ("he_color_selector_update_preview_widget");
	GdkColor color;
	he_color_selector_get_color (selector, &color);
	gtk_widget_modify_bg (selector->priv->preview, GTK_STATE_NORMAL, &color);
	if (!selector->priv->updating)
	{
		selector->priv->updating = TRUE;
		hildon_entry_set_text (HILDON_ENTRY (selector->priv->colorName), gdk_color_to_string (&color));
		selector->priv->updating = FALSE;
	}
}

/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * he_color_selector_new:
 *
 * Creates a new #HeColorSelector
 *
 * Returns: a new #HeColorSelector
 *
 * Since: 2.2
 **/
GtkWidget *
he_color_selector_new ()
{
  //g_warning ("he_color_selector_new");
  return g_object_new (HE_TYPE_COLOR_SELECTOR, NULL);
}

/**
 * he_color_selector_new_with_default_color:
 * @default_color: a GdkColor to initialize with
 *
 * Creates a new #HeColorSelector with a specific color.
 *
 * Returns: a new #HeColorSelector
 *
 * Since: 2.2
 **/
GtkWidget *
he_color_selector_new_with_default_color (GdkColor *default_color)
{
  //g_warning ("he_color_selector_new_with_default_color");
  GtkWidget *selector;
  
  selector = g_object_new (HE_TYPE_COLOR_SELECTOR, NULL);
  he_color_selector_set_color (HE_COLOR_SELECTOR (selector), default_color);

  return selector;
}

/**
 * he_color_selector_set_color:
 * @selector: the #HeColorSelector
 * @color: the color to set
 * 
 * Sets the current active color on the #HeColorSelector widget
 * 
 * Since: 2.2
 * 
 **/
gboolean
he_color_selector_set_color (HeColorSelector *selector,
                                          GdkColor *color)
{
	//g_warning ("he_color_selector_set_color");
	gint red = color->red;
	gint green = color->green;
	gint blue = color->blue;
	gchar *name = gdk_color_to_string (color);
	
	gtk_range_set_value (GTK_RANGE (selector->priv->redSlider), red/65535.0);
	gtk_range_set_value (GTK_RANGE (selector->priv->greenSlider), green/65535.0);
	gtk_range_set_value (GTK_RANGE (selector->priv->blueSlider), blue/65535.0);
	if (!selector->priv->updating)
	{
		selector->priv->updating = TRUE;
		hildon_entry_set_text (HILDON_ENTRY (selector->priv->colorName), name);
		selector->priv->updating = FALSE;
	}
	g_free (name);
	return TRUE;
}

/**
 * he_color_selector_select_red:
 * @selector: the #HeColorSelector
 * @red:  the intensity of red (0 to 65535) to select
 *
 * Sets the current red color portion on the #HeColorSelector widget
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean
he_color_selector_select_red (HeColorSelector * selector,
                                          guint red)
{
  gint color = CLAMP (red, 0, 65535);
  gtk_range_set_value (GTK_RANGE (selector->priv->redSlider), color/65535);

  return TRUE;
}

/**
 * he_color_selector_select_green:
 * @selector: the #HeColorSelector
 * @green:  the intensity of green (0 to 65535) to select
 *
 * Sets the current green color portion on the #HeColorSelector widget
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean
he_color_selector_select_green (HeColorSelector * selector,
                                          guint green)
{
  gint color = CLAMP (green, 0, 65535);
  gtk_range_set_value (GTK_RANGE (selector->priv->greenSlider), color/65535);

  return TRUE;
}

/**
 * he_color_selector_select_blue:
 * @selector: the #HeColorSelector
 * @blue:  the intensity of blue (0 to 65535) to select
 *
 * Sets the current blue color portion on the #HeColorSelector widget
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean
he_color_selector_select_blue (HeColorSelector * selector,
                                          guint blue)
{
  gint color = CLAMP (blue, 0, 65535);
  gtk_range_set_value (GTK_RANGE (selector->priv->blueSlider), color/65535);

  return TRUE;
}

/**
 * he_color_selector_get_color:
 * @selector: the #HeColortSelector
 * @color: a GdkColor to store the color into
 *
 * Gets the current active color on the #HildonColorSelector widget
 *
 * Since: 2.2
 **/
void
he_color_selector_get_color (HeColorSelector * selector,
                                   GdkColor *color)
{
  //g_warning ("he_color_selector_get_color");
  color->red = gtk_range_get_value (GTK_RANGE (selector->priv->redSlider)) * 65535;
  color->green = gtk_range_get_value (GTK_RANGE (selector->priv->greenSlider)) * 65535;
  color->blue = gtk_range_get_value (GTK_RANGE (selector->priv->blueSlider)) * 65535;
}
