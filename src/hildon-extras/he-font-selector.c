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
 * SECTION:he-font-selector
 * @short_description: A widget to select a font.
 *
 * #HeFontSelector is a font widget with multiple columns. Users
 * can choose a font by selecting values in the family, style and size
 * columns.
 *
 * The currently selected family can be altered with
 * he_font_selector_select_family().  The style can be selected
 * with he_font_selection_select_style().  The size can be selected
 * with he_font_selection_select_size().
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <langinfo.h>

#include <hildon/hildon-entry.h>

#include "he-font-selector.h"

#define HE_FONT_SELECTOR_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_FONT_SELECTOR, HeFontSelectorPrivate))

#define _(String) dgettext("hildon-libs", String)

enum
{
  COLUMN_STRING,
  COLUMN_INT,
  N_COLUMNS
};

enum
{
  FAMILY,
  SIZE
};

struct _HeFontSelectorPrivate
{
  GtkTreeModel *family_model;
  GtkTreeModel *size_model;
  
  GtkWidget *familySelector;
  GtkWidget *sizeSelector;
  
  GtkWidget *boldButton;
  GtkWidget *italicButton;
  
  GtkWidget *preview;
};

static GObject * he_font_selector_constructor (GType                  type,
                                                   guint                  n_construct_properties,
                                                   GObjectConstructParam *construct_properties);
static void he_font_selector_finalize (GObject * object);

/* private functions */
static GtkTreeModel *he_font_selector_create_family_model (HeFontSelector * selector);
static GtkTreeModel *he_font_selector_create_size_model (HeFontSelector * selector);

static void he_font_selector_change_selector_selection (HildonTouchSelector *s, gint column, HeFontSelector *selector);
static void he_font_selector_toggle_style_buttons (GtkButton *button, HeFontSelector *selector);
static void he_font_selector_update_preview_widget (HeFontSelector *selector);
static void he_font_selector_class_init (HeFontSelectorClass * class);
static void he_font_selector_init (HeFontSelector * selector);

static gpointer                                 parent_class = NULL;

/**
  * he_font_selector_get_type:
  *
  * Initializes and returns the type of a he font selector.
  *
  * Returns: GType of #HeFontSelector.
  */
 GType G_GNUC_CONST
 he_font_selector_get_type                    (void)
 {
     static GType font_selector_type = 0;
	 font_selector_type = g_type_from_name ("HeFontSelector");
 
     if (! font_selector_type)
     {
         static const GTypeInfo font_selector_info =
         {
             sizeof (HeFontSelectorClass),
             NULL,           /* base_init */
             NULL,           /* base_finalize */
             (GClassInitFunc) he_font_selector_class_init,
             NULL,           /* class_finalize */
             NULL,           /* class_data */
             sizeof (HeFontSelector),
             0,              /* n_preallocs */
             (GInstanceInitFunc) he_font_selector_init,
         };
 
         font_selector_type = g_type_register_static (GTK_TYPE_VBOX, "HeFontSelector",
                 &font_selector_info, 0);
     }
 
     return font_selector_type;
 }

static void
he_font_selector_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  HeFontSelectorPrivate *priv = HE_FONT_SELECTOR (object)->priv;

  switch (prop_id)
  {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
he_font_selector_get_property (GObject      *object,
                                   guint         prop_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
  HeFontSelectorPrivate *priv = HE_FONT_SELECTOR (object)->priv;

  switch (prop_id)
  {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
he_font_selector_class_init (HeFontSelectorClass * class)
{
  //g_warning ("he_font_selector_class_init");
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  gobject_class = (GObjectClass *) class;
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  container_class = (GtkContainerClass *) class;
  
  parent_class = g_type_class_peek_parent (class);

  g_type_class_add_private (object_class, sizeof (HeFontSelectorPrivate));
}

static void
he_font_selector_init (HeFontSelector * selector)
{
  //g_warning ("he_font_selector_init");
  selector->priv = HE_FONT_SELECTOR_GET_PRIVATE (selector);
  selector->priv->familySelector = hildon_touch_selector_new ();
  selector->priv->sizeSelector = hildon_touch_selector_new ();
  
  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  HildonTouchSelectorColumn *column = NULL;
  selector->priv->preview = hildon_entry_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
  GtkWidget *previewLabel = gtk_label_new ("Font preview");
  hildon_entry_set_text (HILDON_ENTRY (selector->priv->preview), "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
  gtk_widget_set_sensitive (selector->priv->preview, FALSE);

  selector->priv->family_model = he_font_selector_create_family_model (selector);
  selector->priv->size_model = he_font_selector_create_size_model (selector);
  
  gtk_box_set_homogeneous (GTK_BOX (selector), FALSE);
  
  GtkWidget *buttonbox = gtk_hbox_new (TRUE, 0);
  selector->priv->boldButton = hildon_gtk_toggle_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
  gtk_container_add (GTK_CONTAINER (buttonbox), selector->priv->boldButton);
  g_signal_connect (selector->priv->boldButton, "toggled", G_CALLBACK (he_font_selector_toggle_style_buttons), selector);
  GtkWidget *boldLabel = gtk_label_new ("B");
  gtk_label_set_markup (GTK_LABEL (boldLabel), "<b>B</b>");
  gtk_container_add (GTK_CONTAINER (selector->priv->boldButton), boldLabel);
  selector->priv->italicButton = hildon_gtk_toggle_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
  gtk_container_add (GTK_CONTAINER (buttonbox), selector->priv->italicButton);
  g_signal_connect (selector->priv->italicButton, "toggled", G_CALLBACK (he_font_selector_toggle_style_buttons), selector);
  GtkWidget *italicLabel = gtk_label_new ("I");
  gtk_label_set_markup (GTK_LABEL (italicLabel), "<i>I</i>");
  gtk_container_add (GTK_CONTAINER (selector->priv->italicButton), italicLabel);
  gtk_widget_set_size_request (buttonbox, 128, -1);
  
  column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector->priv->familySelector),
                                                     selector->priv->family_model, FALSE);
  hildon_touch_selector_column_set_text_column (column, 0);
  
  column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector->priv->sizeSelector),
                                                     selector->priv->size_model, TRUE);
  hildon_touch_selector_column_set_text_column (column, 0);
  
  g_signal_connect (selector->priv->familySelector, "changed", G_CALLBACK (he_font_selector_change_selector_selection), selector);
  g_signal_connect (selector->priv->sizeSelector, "changed", G_CALLBACK (he_font_selector_change_selector_selection), selector);
  
  gtk_box_pack_start (GTK_BOX (hbox), buttonbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), selector->priv->preview, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (selector), hbox, FALSE, FALSE, 0);
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), selector->priv->familySelector, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), selector->priv->sizeSelector, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (selector), hbox, TRUE, TRUE, 0);
  
  gtk_widget_set_size_request (selector->priv->familySelector, 300, -1);
  gtk_widget_set_size_request (selector->priv->sizeSelector, 100, -1);
  gtk_widget_show_all (GTK_WIDGET (selector));

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (selector), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (selector), FALSE);
}

static void
he_font_selector_finalize (GObject * object)
{
  //g_warning ("he_font_selector_finalize");
  HeFontSelector *selector = NULL;

  selector = HE_FONT_SELECTOR (object);

  if (G_OBJECT_CLASS (parent_class)->finalize)
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */

static GtkTreeModel *
he_font_selector_create_size_model (HeFontSelector * selector)
{
  //g_warning ("he_font_selector_create_size_model");
  GtkListStore *store_size = NULL;
  gint i = 0;
  gchar *label;
  GtkTreeIter iter;
  gint font_sizes[] = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32 };

  store_size = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  for (i = 0; i < G_N_ELEMENTS (font_sizes); i++) {
    label = g_strdup_printf ("%d", font_sizes[i]);

    gtk_list_store_append (store_size, &iter);
    gtk_list_store_set (store_size, &iter, COLUMN_STRING, label,
                        COLUMN_INT, i,
                        -1);
  }
  g_free (label);

  return GTK_TREE_MODEL (store_size);
}

static GtkTreeModel *
he_font_selector_create_family_model (HeFontSelector * selector)
{
  //g_warning ("he_font_selector_create_family_model");
  GtkTreeIter iter;
  gint i = 0;
  GtkListStore *store_family = NULL;
  gchar *label;
  PangoFontFamily **families;
  gint n_families;

  store_family = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  pango_context_list_families (gtk_widget_get_pango_context
             (GTK_WIDGET (selector)), &families,
             &n_families);
  for (i = 0; i < n_families; i++) {
    label = g_strdup (pango_font_family_get_name (families[i]));

    gtk_list_store_append (store_family, &iter);
    gtk_list_store_set (store_family, &iter, COLUMN_STRING, label,
                        COLUMN_INT, i,
                        -1);
  }
  g_free (label);

  return GTK_TREE_MODEL (store_family);
}

static void
he_font_selector_change_selector_selection (HildonTouchSelector *s, gint column, HeFontSelector *selector)
{
	//g_warning ("he_font_selector_change_selector_selection");
	he_font_selector_update_preview_widget (selector);
}

static void
he_font_selector_toggle_style_buttons (GtkButton *button, HeFontSelector *selector)
{
	//g_warning ("he_font_selector_toggle_style_buttons");
	he_font_selector_update_preview_widget (selector);
}

static void
he_font_selector_update_preview_widget (HeFontSelector *selector)
{
	//g_warning ("he_font_selector_update_preview_widget");
	gchar *font_desc = he_font_selector_get_font (selector);
	gtk_widget_modify_font (selector->priv->preview, pango_font_description_from_string (font_desc));
	g_free (font_desc);
}

/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * he_font_selector_new:
 *
 * Creates a new #HeFontSelector
 *
 * Returns: a new #HeFontSelector
 *
 * Since: 2.2
 **/
GtkWidget *
he_font_selector_new ()
{
  //g_warning ("he_font_selector_new");
  return g_object_new (HE_TYPE_FONT_SELECTOR, NULL);
}

/**
 * he_font_selector_new_with_default_font:
 * @default_font: the string representation of a font
 *
 * Creates a new #HeFontSelector with a specific font.
 *
 * Returns: a new #HeFontSelector
 *
 * Since: 2.2
 **/
GtkWidget *
he_font_selector_new_with_default_font (gchar *default_font)
{
  //g_warning ("he_font_selector_new_with_default_font");
  GtkWidget *selector;
  
  selector = g_object_new (HE_TYPE_FONT_SELECTOR, NULL);
  he_font_selector_set_font (HE_FONT_SELECTOR (selector), default_font);

  return selector;
}

/**
 * he_font_selector_set_font:
 * @selector: the #HeFontSelector
 * @font_string: the font to set
 * 
 * Sets the current active font on the #HeFontSelector widget
 * 
 * Since: 2.2
 * 
 **/
gboolean
he_font_selector_set_font (HeFontSelector *selector,
                                          gchar *font_string)
{
	//g_warning ("he_font_selector_set_font");
	PangoFontDescription *font_desc = pango_font_description_from_string (font_string);
	GtkTreeIter iter;
	gboolean exists=FALSE;
	gboolean found=FALSE;
	gchar *comp="";
	
	gchar *family = g_strdup (pango_font_description_get_family (font_desc));
	if (family == NULL) {
		family = "Sans";
	}

	exists = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->priv->family_model), &iter);
	while (exists && !found) {
		gtk_tree_model_get (GTK_TREE_MODEL (selector->priv->family_model), &iter, COLUMN_STRING, &comp, -1);
		found = g_str_equal (family, comp);
		if (found)
			hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector->priv->familySelector), 0, &iter, TRUE);
		exists = gtk_tree_model_iter_next (GTK_TREE_MODEL (selector->priv->family_model), &iter);
	}
	g_free (comp);	
	
	gint size =	pango_font_description_get_size (font_desc) / PANGO_SCALE;
	if (size == 0) {
		size = 18;
	}
	gchar *sizeString = g_strdup_printf ("%d", size);
	found = FALSE;
	exists = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->priv->size_model), &iter);
	while (exists && !found) {
		gtk_tree_model_get (GTK_TREE_MODEL (selector->priv->size_model), &iter, COLUMN_STRING, &comp, -1);
		found = g_str_equal (sizeString, comp);
		if (found)
			hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector->priv->sizeSelector), 0, &iter, TRUE);
		exists = gtk_tree_model_iter_next (GTK_TREE_MODEL (selector->priv->size_model), &iter);
	}
	g_free (comp);
	
	PangoStyle style = pango_font_description_get_style (font_desc);
	if (style == PANGO_STYLE_ITALIC) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (selector->priv->italicButton), TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (selector->priv->italicButton), FALSE);
	}
	
	PangoWeight weight = pango_font_description_get_weight (font_desc);
	if (weight == PANGO_WEIGHT_BOLD) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (selector->priv->boldButton), TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (selector->priv->boldButton), FALSE);
	}
	return TRUE;
}

/**
 * he_font_selector_select_family:
 * @selector: the #HeFontSelector
 * @family:  the family to select
 *
 * Sets the current active font on the #HeFontSelector widget
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean
he_font_selector_select_family (HeFontSelector * selector,
                                          gchar *family)
{
  GtkTreeIter iter;
  gboolean exists = FALSE;
  gboolean found = FALSE;
  gchar *comp;
  exists = gtk_tree_model_get_iter_first (selector->priv->family_model, &iter);
  while (exists && !found)
  {
    gtk_tree_model_get (selector->priv->family_model, &iter, COLUMN_STRING, &comp, -1);
    found = g_str_equal (family, comp);
    if (found)
    {
    	hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector->priv->familySelector),
                                     0, &iter,
                                     FALSE);
	}
    exists = gtk_tree_model_iter_next (selector->priv->family_model, &iter);
  }
  g_free (comp);

  return found;
}

/**
 * he_font_selector_select_size:
 * @selector: the #HeFontSelector
 * @size:  the size to select
 *
 * Sets the current active font on the #HeFontSelector widget
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean
he_font_selector_select_size (HeFontSelector * selector,
                                          guint size)
{
  GtkTreeIter iter;

  gboolean exists = FALSE;
  gboolean found = FALSE;
  gchar *comp;
  gchar *sizeString = g_strdup_printf ("%d", size);
  exists = gtk_tree_model_get_iter_first (selector->priv->size_model, &iter);
  while (exists && !found)
  {
    gtk_tree_model_get (selector->priv->size_model, &iter, COLUMN_STRING, &comp, -1);
    found = g_str_equal (sizeString, comp);
    if (found)
    {
    	hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector->priv->sizeSelector),
                                     0, &iter,
                                     FALSE);
	}
    exists = gtk_tree_model_iter_next (selector->priv->size_model, &iter);
  }
  g_free (sizeString);
  g_free (comp);

  return found;
}

/**
 * he_font_selector_get_font:
 * @selector: the #HeFontSelector
 *
 * Gets the current active date on the #HildonDateSelector widget
 *
 * Since: 2.2
 * 
 * Returns: A string of text representing the current font
 **/
gchar *
he_font_selector_get_font (HeFontSelector * selector)
{
  //g_warning ("he_font_selector_get_font");
  GtkTreeIter iter;
  gchar *familyString=NULL;
  gchar *styleString=NULL;
  gchar *sizeString=NULL;
  gchar *fontString=NULL;

  familyString = hildon_touch_selector_get_current_text (HILDON_TOUCH_SELECTOR (selector->priv->familySelector));

  styleString = g_strconcat (" ", NULL);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (selector->priv->boldButton)))
	styleString = g_strconcat (styleString, "bold ", NULL);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (selector->priv->italicButton)))
	styleString = g_strconcat (styleString, "italic ", NULL);
	
  sizeString = hildon_touch_selector_get_current_text (HILDON_TOUCH_SELECTOR (selector->priv->sizeSelector));

  fontString = g_strconcat (familyString, styleString, sizeString, NULL);
  g_free (familyString);
  g_free (styleString);
  g_free (sizeString);
  
  return fontString;

}
