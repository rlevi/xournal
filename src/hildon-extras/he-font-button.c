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
 * SECTION:he-font-button
 * @short_description: A widget to open HeFontDialog
 * @see_also: #HeFontSelector, #HeFontDialog
 *
 * #HeFontButton is a widget to open a HeFontDialog.
 * The selected font is shown in the button.
 * The selected font is a property of the button.
 * The property name is "font" and its type is gchar*.
 *
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <langinfo.h>

#include "he-font-button.h"
#include "he-font-dialog.h"

#define HE_FONT_BUTTON_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_FONT_BUTTON, HeFontButtonPrivate))

#define _(String) dgettext("hildon-libs", String)

struct _HeFontButtonPrivate
{
	GtkWidget *dialog;
	GtkWidget *hbox;
	GtkWidget *fontLabel;
	GtkWidget *sizeLabel;
	gboolean popped;

	gchar *font;
	GdkGC *gc;
};

enum
{
    SETUP_DIALOG,
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_FONT,
    PROP_POPUP_SHOWN
};

static GObject * he_font_button_constructor (GType                  type,
                                                   guint                  n_construct_properties,
                                                   GObjectConstructParam *construct_properties);
static void he_font_button_finalize (GObject * object);

static void he_font_button_clicked (GtkButton *button);
static void he_font_button_class_init (HeFontButtonClass * class);
static void he_font_button_init (HeFontButton *fb);

static gpointer                                 parent_class = NULL;
static guint                                    signals [LAST_SIGNAL] = { 0, };

/* private functions */
/**
  * he_font_button_get_type:
  *
  * Initializes and returns the type of a he font button.
  *
  * Returns: GType of #HeFontButton.
  */
 GType G_GNUC_CONST
 he_font_button_get_type                    (void)
 {
     static GType font_button_type = 0;
	 font_button_type = g_type_from_name ("HeFontButton");
 
     if (! font_button_type)
     {
         static const GTypeInfo font_button_info =
         {
             sizeof (HeFontButtonClass),
             NULL,           /* base_init */
             NULL,           /* base_finalize */
             (GClassInitFunc) he_font_button_class_init,
             NULL,           /* class_finalize */
             NULL,           /* class_data */
             sizeof (HeFontButton),
             0,              /* n_preallocs */
             (GInstanceInitFunc) he_font_button_init,
         };
 
         font_button_type = g_type_register_static (GTK_TYPE_BUTTON, "HeFontButton",
                 &font_button_info, 0);
     }
 
     return font_button_type;
 }

static void
he_font_button_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	//g_warning ("he_font_button_set_property");
  HeFontButton *cb = HE_FONT_BUTTON (object);
  HeFontButtonPrivate *priv = HE_FONT_BUTTON_GET_PRIVATE (cb);

  switch (prop_id)
  {
  case PROP_FONT:
    priv->font = g_value_dup_string (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
he_font_button_get_property (GObject      *object,
                                   guint         prop_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
	//g_warning ("he_font_button_get_property");
  HeFontButton *cb = HE_FONT_BUTTON (object);
  HeFontButtonPrivate *priv = HE_FONT_BUTTON_GET_PRIVATE (cb);

  switch (prop_id)
  {
    case PROP_FONT:
      g_value_set_string (value, priv->font);
      break;
    case PROP_POPUP_SHOWN:
      g_value_set_boolean (value, priv->popped);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
he_font_button_class_init (HeFontButtonClass * class)
{
  //g_warning ("he_font_button_class_init");
  GObjectClass *gobject_class;
  GtkButtonClass *button_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass *) class;
  button_class = (GtkButtonClass *) class;
  widget_class = (GtkWidgetClass *) class;
  
  parent_class = g_type_class_peek_parent (class);

  /* GObject */
  //gobject_class->finalize = he_font_button_finalize;
  gobject_class->get_property = he_font_button_get_property;
  gobject_class->set_property = he_font_button_set_property;
  button_class->clicked = he_font_button_clicked;

  /* GtkWidget */

  /* GtkContainer */

  /* properties */

  g_object_class_install_property (
    gobject_class,
    PROP_FONT,
    g_param_spec_string (
      "font",
      "Current Font",
      "The selected font",
	  "Sans 18",
	  G_PARAM_READWRITE));

  g_object_class_install_property (
    gobject_class,
    PROP_POPUP_SHOWN,
    g_param_spec_boolean (
      "popup-shown",
      "IsPopped",
      "If the font dialog is popped up",
      FALSE,
	  G_PARAM_READABLE));

  /* signals */

  g_type_class_add_private (button_class, sizeof (HeFontButtonPrivate));
}

static void
he_font_button_init (HeFontButton *fb)
{
  //g_warning ("he_font_button_init");
  fb->priv = HE_FONT_BUTTON_GET_PRIVATE (fb);
  fb->priv->dialog=NULL;
  
  fb->priv->hbox = gtk_hbox_new (FALSE, 0);
  fb->priv->fontLabel = gtk_label_new ("Sans");
  fb->priv->sizeLabel = gtk_label_new ("18");
  gtk_box_pack_start (GTK_BOX (fb->priv->hbox), fb->priv->fontLabel, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (fb->priv->hbox), gtk_vseparator_new (), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (fb->priv->hbox), fb->priv->sizeLabel, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (fb), fb->priv->hbox);
  gtk_widget_show_all (GTK_WIDGET (fb));

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (fb), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (fb), FALSE);
}

static void
he_font_button_finalize (GObject * object)
{
  //g_warning ("he_font_button_finalize");
  HeFontButtonPrivate *priv = HE_FONT_BUTTON_GET_PRIVATE (object);

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
he_font_button_clicked (GtkButton *button)
{
	//g_warning ("he_font_button_clicked");
	HeFontButton *fb = HE_FONT_BUTTON (button);
	HeFontDialog *f_dialog;
	f_dialog = (HeFontDialog *) fb->priv->dialog;
	if (! f_dialog)
	{
		GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (fb));
		fb->priv->dialog = he_font_dialog_new ();
		he_font_dialog_set_font (HE_FONT_DIALOG (fb->priv->dialog), fb->priv->font);
		f_dialog = HE_FONT_DIALOG (fb->priv->dialog);
		if (parent)
			gtk_window_set_transient_for (GTK_WINDOW (f_dialog), GTK_WINDOW (parent));
	}
	he_font_dialog_set_font (f_dialog, fb->priv->font);
	
	fb->priv->popped = TRUE;

	if (gtk_dialog_run (GTK_DIALOG (f_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *font = he_font_dialog_get_font (f_dialog);
		he_font_button_set_font (fb, font);
		g_free (font);
	}
	gtk_widget_hide (GTK_WIDGET (fb->priv->dialog));
	fb->priv->popped = FALSE;
}


/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * he_font_button_new:
 *
 * Creates a new #HeFontButton
 *
 * Returns: a new #HeFontButton
 *
 * Since: 2.2
 **/
GtkWidget *
he_font_button_new ()
{
  //g_warning ("he_font_button_new");
  return g_object_new (HE_TYPE_FONT_BUTTON, NULL);
}

/**
 * he_font_button_new_with_font:
 * @font: the font to initialize to
 *
 * Creates a new #HeFontButton with a specific font.
 *
 * Returns: a new #HeFontButton
 *
 * Since: 2.2
 **/
GtkWidget *
he_font_button_new_with_font (const gchar *font_string)
{
  //g_warning ("he_font_button_new_with_font");
  GtkWidget *fb = g_object_new (HE_TYPE_FONT_BUTTON, "font", font_string, NULL);
  he_font_button_set_font (HE_FONT_BUTTON (fb), font_string);
  return fb;
}

/**
 * he_font_button_set_font:
 * @cb: the #HeFontButton
 * @font: the font to set
 * 
 * Sets the current font on the #HeFontButton widget
 * 
 * Since: 2.2
 * 
 **/
void
he_font_button_set_font (HeFontButton *fb,
                          const gchar *font_string)
{
	//g_warning ("he_font_button_set_font");
	g_return_if_fail (HE_IS_FONT_BUTTON (fb));
	
	g_object_set (G_OBJECT (fb), "font", font_string, NULL);
	const PangoFontDescription *pango_desc = pango_font_description_from_string (font_string);
	gchar *fontLabelString = g_strdup (pango_font_description_get_family (pango_desc));
	PangoWeight pango_weight = pango_font_description_get_weight (pango_desc);
	if (pango_weight == PANGO_WEIGHT_BOLD)
		fontLabelString = g_strconcat (fontLabelString, " bold", NULL);
	PangoStyle pango_style = pango_font_description_get_style (pango_desc);
	if (pango_style == PANGO_STYLE_ITALIC)
		fontLabelString = g_strconcat (fontLabelString, " italic", NULL);
	gint size = pango_font_description_get_size (pango_desc)/PANGO_SCALE;
	gchar *sizeLabelString = g_strdup_printf ("%d", size);
	gtk_label_set_text (GTK_LABEL (fb->priv->fontLabel), fontLabelString);
	gtk_label_set_text (GTK_LABEL (fb->priv->sizeLabel), sizeLabelString);
	g_free (fontLabelString);
	g_free (sizeLabelString);
}

/**
 * he_font_button_get_popup_down:
 * @cb: the #HeFontButton
 *
 * This function checks if the font button has the font
 * dialog currently popped-up
 *
 * Since: 2.2
 *
 * Returns: %TRUE if the dialog is popped-up (visible to user)
 **/
gboolean
he_font_button_get_popup_shown (HeFontButton * fb)
{
	//g_warning ("he_font_button_get_popup_shown");
  g_return_val_if_fail (HE_IS_FONT_BUTTON (fb), FALSE);
  
  return fb->priv->popped;
}

/**
 * he_font_button_popdown:
 * @cb: the #HeFontButton
 *
 * If the font dialog is currently popped-up (visible)
 * it will be popped-down (hidden).
 *
 * Since: 2.2
 **/
void
he_font_button_popdown (HeFontButton * fb)
{
	//g_warning ("he_font_button_popdown");
	g_return_if_fail (HE_IS_FONT_BUTTON (fb));
	
	if (fb->priv->popped && fb->priv->dialog)
	{
		gtk_dialog_response (GTK_DIALOG (fb->priv->dialog), GTK_RESPONSE_CANCEL);
	}
}

/**
 * he_font_button_get_font:
 * @cb: the #HeFontButton
 *
 * @returns: a gchar* string representing the font
 * Since: 2.2
 **/
gchar*
he_font_button_get_font (HeFontButton *fb)
{
  //g_warning ("he_font_button_get_font");
  g_return_if_fail (HE_IS_FONT_BUTTON (fb));
  
  return fb->priv->font;

}

