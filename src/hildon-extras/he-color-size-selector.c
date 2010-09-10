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

/*
 * SECTION:he-color-size-selector
 * @short_description: A widget to select a color and a size.
 *
 * #HeColorSizeSelector is a color/size widget with multiple sliders.
 * Users can choose a color by selecting values on the red, green and blue
 * sliders and a size by selecting a value for the size slider.
 *
 * The current red can be altered with he_color_size_selector_set_red().
 * The current green can be altered with he_color_size_selector_set_green().
 * The current blue can be selected with he_color_size_selector_set_blue().
 * Tue current size can be selected with he_color_size_selector_set_size().
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

#include "he-color-size-selector.h"

#define HE_COLOR_SIZE_SELECTOR_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), HE_TYPE_COLOR_SIZE_SELECTOR, HeColorSizeSelectorPrivate))

#define _(String) dgettext("hildon-libs", String)

struct _HeColorSizeSelectorPrivate
{
	GtkWidget *redSlider;
	GtkWidget *redValue;
	GtkWidget *greenSlider;
	GtkWidget *greenValue;
	GtkWidget *blueSlider;
	GtkWidget *blueValue;
	GtkWidget *customColors[4];

	GtkWidget *sizeSlider;
	GtkWidget *presetSizes[4];

	GtkWidget *previewArea;

	gboolean updating;
};

/* private functions */
static void he_color_size_selector_slider_changed (GtkRange *range, HeColorSizeSelector *selector);
static void he_color_size_selector_value_changed (GtkEditable *editable, HeColorSizeSelector *selector);
static void he_color_size_selector_update_preview_area (HeColorSizeSelector *selector);
static void he_color_size_selector_class_init (HeColorSizeSelectorClass *class);
static void he_color_size_selector_init (HeColorSizeSelector *selector);
static void he_color_size_selector_slider_preset_clicked (GtkWidget *drawable, HeColorSizeSelector *selector);
static void he_color_size_selector_color_preset_clicked (GtkWidget *drawable, HeColorSizeSelector *selector);
static void he_color_size_selector_text_changed (GtkEditable *editable, HeColorSizeSelector *selector);

static gpointer parent_class = NULL;

/*
 * he_color_size_selector_get_type:
 *
 * Initializes and returns the type of the Hildon Extras color and size selector.
 *
 * Returns: GType of #HeColorSizeSelector.
 */
GType G_GNUC_CONST
he_color_size_selector_get_type (void)
{
	static GType color_size_selector_type = 0;

	g_warning ("_get_type");

	color_size_selector_type = g_type_from_name ("HeColorSizeSelector");

	if (!color_size_selector_type) {
		static const GTypeInfo color_size_selector_info = 
		{
			sizeof (HeColorSizeSelectorClass),
			NULL,    /* base_init */
			NULL,    /* base_finalize */
			(GClassInitFunc) he_color_size_selector_class_init,
			NULL,    /* class_finalize */
			NULL,    /* class_data */
			sizeof (HeColorSizeSelector),
			0,       /* n_preallocs */
			(GInstanceInitFunc) he_color_size_selector_init
		};

		color_size_selector_type = g_type_register_static (GTK_TYPE_VBOX, "HeColorSizeSelector",
				&color_size_selector_info, 0);
	}

	g_warning ("end _get_type");

	return color_size_selector_type;
}

static void
he_color_size_selector_class_init (HeColorSizeSelectorClass *class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	g_warning ("_class_init");

	gobject_class   = (GObjectClass *) class;
	object_class    = (GtkObjectClass *) class;
	widget_class    = (GtkWidgetClass *) class;
	container_class = (GtkContainerClass *) class;

	parent_class = g_type_class_peek_parent (class);

	g_type_class_add_private (object_class, sizeof (HeColorSizeSelectorPrivate));
}

gboolean
preview_area_exposed (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GdkColor color;

	g_warning ("exposing");
	gdk_color_parse ("white", &color);
//	gtk_widget_modify_fg (widget, GTK_STATE_NORMAL, &color);
	gdk_draw_arc (widget->window, /* Drawable object */
		widget->style->fg_gc[GTK_WIDGET_STATE (widget)], /* GC */
		TRUE, /* Not filled - it's a black bg, so let's only draw a light circle */
		0, 0, /* top left of the drawable area */
		50, 50,
		0, 64*360); /* full circle */
	g_warning ("done exposing");

	return TRUE;
}

static void
he_color_size_selector_init (HeColorSizeSelector *selector)
{
	GtkSizeGroup *colorGroup, *sliderGroup;
	GtkWidget *red, *green, *blue, *slider;
	GtkWidget *hbox, *vbox_rightSide, *vbox_leftSide;
	GtkWidget *hbox_r, *hbox_g, *hbox_b, *vbox_colors, *hbox_presets;
	GtkWidget *hbox_customColors, *vbox_sizeSlider;
	gint i;

	g_warning ("_init");

	selector->priv = HE_COLOR_SIZE_SELECTOR_GET_PRIVATE (selector);

	hbox           = gtk_hbox_new (FALSE, 0);
	vbox_rightSide = gtk_vbox_new (FALSE, 0);
	vbox_leftSide  = gtk_vbox_new (FALSE, 0);

	/* Color sliders */
	g_warning ("color sliders");
	selector->priv->redSlider   = hildon_gtk_hscale_new ();
	selector->priv->greenSlider = hildon_gtk_hscale_new ();
	selector->priv->blueSlider  = hildon_gtk_hscale_new ();
	gtk_widget_set_size_request (selector->priv->redSlider, 200, 100);
	gtk_widget_set_size_request (selector->priv->greenSlider, 200, 100);
	gtk_widget_set_size_request (selector->priv->blueSlider, 200, 100);

	colorGroup = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	red   = hildon_caption_new (colorGroup, "R",   selector->priv->redSlider,   NULL, HILDON_CAPTION_MANDATORY);
	green = hildon_caption_new (colorGroup, "G", selector->priv->greenSlider, NULL, HILDON_CAPTION_MANDATORY);
	blue  = hildon_caption_new (colorGroup, "B",  selector->priv->blueSlider,  NULL, HILDON_CAPTION_MANDATORY);

	/* Color text areas */
	g_warning ("color text areas");
	selector->priv->redValue    = hildon_entry_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
	selector->priv->greenValue  = hildon_entry_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
	selector->priv->blueValue   = hildon_entry_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);

	/* Pack color sliders and text areas */
	g_warning ("pack sliders and text areas");
	hbox_r = gtk_hbox_new (FALSE, 0);
	hbox_g = gtk_hbox_new (FALSE, 0);
	hbox_b = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER(hbox_r), red);
	gtk_container_add (GTK_CONTAINER(hbox_r), selector->priv->redValue);
	gtk_container_add (GTK_CONTAINER(hbox_g), green);
	gtk_container_add (GTK_CONTAINER(hbox_g), selector->priv->greenValue);
	gtk_container_add (GTK_CONTAINER(hbox_b), blue);
	gtk_container_add (GTK_CONTAINER(hbox_b), selector->priv->blueValue);

	vbox_colors = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER(vbox_colors), hbox_r);
	gtk_container_add (GTK_CONTAINER(vbox_colors), hbox_g);
	gtk_container_add (GTK_CONTAINER(vbox_colors), hbox_b);
	/* End packing color sliders */

	/* Custom color buttons */
	g_warning ("custom color buttons");
	hbox_customColors = gtk_hbox_new (FALSE, 0);
	for (i=0;i<4;i++) {
		selector->priv->customColors[i] = gtk_drawing_area_new ();
		gtk_widget_set_size_request (selector->priv->customColors[i], 80, 80);

		gtk_container_add (GTK_CONTAINER(hbox_customColors), selector->priv->customColors[i]);

		g_object_set (G_OBJECT(selector->priv->customColors[i]), "preset_id", i);

		/* a Gtk Drawable doesn't usually handle click events */
		gtk_widget_add_events (selector->priv->customColors[i], GDK_BUTTON_PRESS_MASK);
		g_signal_connect (selector->priv->customColors[i], "clicked",
				G_CALLBACK (he_color_size_selector_color_preset_clicked), selector);

		/* TODO: set property of the colors (color_name) */
	}

	gtk_container_add (GTK_CONTAINER(vbox_colors), hbox_customColors);
	/* End of custom color buttons */

	/* Preview area */
	g_warning ("preview area");
	selector->priv->previewArea = gtk_drawing_area_new ();
	gtk_widget_set_size_request (selector->priv->previewArea, 80, 80);

	g_signal_connect (G_OBJECT(selector->priv->previewArea), "expose_event",
			G_CALLBACK(preview_area_exposed), NULL);

	gtk_container_add (GTK_CONTAINER(vbox_rightSide), selector->priv->previewArea);

	/* Size slider */
	/* Presets */
	g_warning ("presets");
	hbox_presets = gtk_hbox_new (FALSE, 0);
	for (i=0;i<4;i++) {
		GtkWidget *drawable;
		GdkColor color;
		gint top, diameter;

		g_warning ("preset %d", i);
		selector->priv->presetSizes[i] = gtk_drawing_area_new ();
		drawable = selector->priv->presetSizes[i];

		g_object_set (G_OBJECT(drawable), "preset_value", 100/(i+1));

		/* a Gtk Drawable doesn't usually handle click events */
		gtk_widget_add_events (drawable, GDK_BUTTON_PRESS_MASK);
		g_signal_connect (drawable, "clicked",
				G_CALLBACK (he_color_size_selector_slider_preset_clicked), selector);

		gtk_widget_set_size_request (drawable, 50, 50);

		gdk_color_parse ("white", &color);
		gtk_widget_modify_fg (drawable, GTK_STATE_NORMAL, &color);

		diameter = 50/(i+1);
		top = (50-diameter)/2;

		gdk_draw_arc (drawable->window, /* Drawable object */
			drawable->style->fg_gc[GTK_WIDGET_STATE (drawable)], /* GC */
			FALSE, /* Not filled - it's a black bg, so let's only draw a light circle */
			top, top, /* top left of the drawable area */
			diameter, diameter,
			0, 64*360); /* full circle */

		gdk_draw_line (drawable->window,
			drawable->style->fg_gc[GTK_WIDGET_STATE (drawable)], /* GC */
			25, top+diameter,
			25, top+diameter+3);

		gtk_container_add (GTK_CONTAINER(hbox_presets), selector->priv->presetSizes[i]);
	}


	/* The size slider itself */
	g_warning ("size slider");
	selector->priv->sizeSlider  = hildon_gtk_hscale_new ();
	sliderGroup = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	slider = hildon_caption_new (sliderGroup, "Size", selector->priv->sizeSlider, NULL, HILDON_CAPTION_MANDATORY);

	/* Group them together */
	vbox_sizeSlider = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER(vbox_sizeSlider), hbox_presets);
	gtk_container_add (GTK_CONTAINER(vbox_sizeSlider), slider);

	/* End of size slider */

	/* Layout */
	gtk_container_add (GTK_CONTAINER(vbox_leftSide), vbox_colors);
	gtk_container_add (GTK_CONTAINER(vbox_rightSide), vbox_sizeSlider);

	gtk_container_add (GTK_CONTAINER(hbox), vbox_leftSide);
	gtk_container_add (GTK_CONTAINER(hbox), vbox_rightSide);

	gtk_container_add (GTK_CONTAINER(selector), hbox);

	/* TODO: Signals */
	g_signal_connect (selector->priv->redSlider,   "value-changed",
			G_CALLBACK (he_color_size_selector_slider_changed), selector);
	g_signal_connect (selector->priv->greenSlider, "value-changed",
			G_CALLBACK (he_color_size_selector_slider_changed), selector);
	g_signal_connect (selector->priv->blueSlider,  "value-changed",
			G_CALLBACK (he_color_size_selector_slider_changed), selector);

	g_signal_connect (selector->priv->redValue, "changed",
			G_CALLBACK (he_color_size_selector_text_changed), selector);
	g_signal_connect (selector->priv->greenValue, "changed",
			G_CALLBACK (he_color_size_selector_text_changed), selector);
	g_signal_connect (selector->priv->blueValue, "changed",
			G_CALLBACK (he_color_size_selector_text_changed), selector);

	g_signal_connect (selector->priv->sizeSlider, "changed",
			G_CALLBACK (he_color_size_selector_text_changed), selector);

	GTK_WIDGET_SET_FLAGS (GTK_WIDGET(selector), GTK_NO_WINDOW);
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET(selector), FALSE);
}

/* TODO: Private methods */
static void
he_color_size_selector_update_preview (HeColorSizeSelector *selector)
{
	GdkColor color;
	gdouble size;
	gchar *red, *green, *blue;

	g_warning ("update_preview");

	he_color_size_selector_get_color (selector, &color);
	he_color_size_selector_get_size  (selector, &size);

	gtk_widget_modify_fg (selector->priv->previewArea, GTK_STATE_NORMAL, &color);

	if (!selector->priv->updating) {
		selector->priv->updating = TRUE;

		/* Update text widgets */
		red   = g_strdup_printf ("#%04x", color.red);
		green = g_strdup_printf ("#%04x", color.green);
		blue  = g_strdup_printf ("#%04x", color.blue);
	
		hildon_entry_set_text (HILDON_ENTRY(selector->priv->redValue),   red);
		hildon_entry_set_text (HILDON_ENTRY(selector->priv->greenValue), green);
		hildon_entry_set_text (HILDON_ENTRY(selector->priv->blueValue),  blue);

		g_free (red);
		g_free (green);
		g_free (blue);

		selector->priv->updating = FALSE;
	}
	/* TODO: update size of circle in the preview area */
}

static void
he_color_size_selector_slider_changed (GtkRange *range, HeColorSizeSelector *selector)
{
	g_warning ("_slider_changed");

	he_color_size_selector_update_preview (selector);
}

static void
he_color_size_selector_text_changed (GtkEditable *editable, HeColorSizeSelector *selector)
{
	GdkColor color;
	gchar color_name[100];

	g_warning ("_text_changed");

	if (!selector->priv->updating) {
		g_sprintf (color_name, "#%04x%04x%04x",
			hildon_entry_get_text (HILDON_ENTRY (selector->priv->redValue)),
			hildon_entry_get_text (HILDON_ENTRY (selector->priv->greenValue)),
			hildon_entry_get_text (HILDON_ENTRY (selector->priv->blueValue)));

		if (gdk_color_parse (color_name, &color)) {
			he_color_size_selector_set_color (selector, &color);
		}
	}
}

static void
he_color_size_selector_color_preset_clicked (GtkWidget *drawable, HeColorSizeSelector *selector)
{
	g_warning ("_color_preset_clicked");

}

static void
he_color_size_selector_slider_preset_clicked (GtkWidget *drawable, HeColorSizeSelector *selector)
{
	gdouble preset_value;

	g_warning ("_slider_preset_clicked");

	g_object_get (G_OBJECT(drawable), "preset_value", &preset_value);

	he_color_size_selector_set_size (selector, preset_value/100.0);
}

/* Public method */

/**
 * he_color_size_selector_new:
 *
 * Creates a new #HeColorSizeSelector
 *
 * Returns: a new #HeColorSizeSelector
 *
 **/
HeColorSizeSelector *
he_color_size_selector_new ()
{
	g_warning ("_new");

	return g_object_new (HE_TYPE_COLOR_SIZE_SELECTOR, NULL);
}

/**
 * he_color_size_selector_set_size:
 * @selector: the #HeColorSizeSelector
 * @size: a size (gdouble)
 *
 * Sets the current size on the #HeColorSizeSelector widget
 * The size must be between 0 and 1
 *
 **/
gboolean
he_color_size_selector_set_size (HeColorSizeSelector *selector,
		gdouble size)
{
	g_warning ("_set_size");

	gtk_range_set_value (GTK_RANGE (selector->priv->sizeSlider), size);

	return TRUE;
}

/**
 * he_color_size_selector_get_size:
 * @selector: the #HeColorSizeSelector
 * @size: the current size on the #HeColorSizeSelector widget
 *
 * Returns: the size of the #HeColorSizeSelector (between 0 and 1)
 **/
void
he_color_size_selector_get_size (HeColorSizeSelector *selector,
		gdouble *size)
{
	g_warning ("_get_size");

	*size = gtk_range_get_value (GTK_RANGE(selector->priv->sizeSlider));
}

/**
 * he_color_size_selector_set_color:
 * @selector: the #HeColorSizeSelector
 * @color: a GkdColor
 *
 * Sets the current color on the #HeColorSizeSelector widget
 * 
 **/
gboolean
he_color_size_selector_set_color (HeColorSizeSelector *selector,
		GdkColor *color)
{
	gint red   = color->red;
	gint green = color->green;
	gint blue  = color->blue;

	gchar *red_text   = g_strdup_printf ("#%04x", red);
	gchar *green_text = g_strdup_printf ("#%04x", green);
	gchar *blue_text  = g_strdup_printf ("#%04x", blue);

	g_warning ("_set_color");

	gtk_range_set_value (GTK_RANGE (selector->priv->redSlider),   red/65535.0);
	gtk_range_set_value (GTK_RANGE (selector->priv->greenSlider), green/65535.0);
	gtk_range_set_value (GTK_RANGE (selector->priv->blueSlider),  blue/65535.0);

	if (!selector->priv->updating) {
		selector->priv->updating = TRUE;
		hildon_entry_set_text (HILDON_ENTRY (selector->priv->redValue),   red_text);
		hildon_entry_set_text (HILDON_ENTRY (selector->priv->greenValue), green_text);
		hildon_entry_set_text (HILDON_ENTRY (selector->priv->blueValue),  blue_text);
		selector->priv->updating = FALSE;
	}

	g_free (red_text);
	g_free (green_text);
	g_free (blue_text);

	return TRUE;
}

/**
 * he_color_size_selector_get_color:
 * @selector: a #HeColorSizeSelector
 * @color: a GdkColor to store the color into
 *
 * Gets the current color of the #HeColorSizeSelector into @color
 **/
void
he_color_size_selector_get_color (HeColorSizeSelector *selector,
		GdkColor *color)
{
	g_warning ("_get_color");

	color->red   = gtk_range_get_value (GTK_RANGE (selector->priv->redSlider))   * 65535;
	color->green = gtk_range_get_value (GTK_RANGE (selector->priv->greenSlider)) * 65535;
	color->blue  = gtk_range_get_value (GTK_RANGE (selector->priv->blueSlider))  * 65535;
}
