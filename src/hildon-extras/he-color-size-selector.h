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

#ifndef __HE_COLOR_SIZE_SELECTOR_H__
#define __HE_COLOR_SIZE_SELECTOR_H__

G_BEGIN_DECLS

#define HE_TYPE_COLOR_SIZE_SELECTOR \
	(he_color_size_selector_get_type ())

#define HE_COLOR_SIZE_SELECTOR(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	HE_TYPE_COLOR_SIZE_SELECTOR, HeColorSizeSelector))

#define HE_COLOR_SIZE_SELECTOR_CLASS(vtable) \
	(G_TYPE_CHECK_CLASS_CAST ((vtable), \
	HE_TYPE_COLOR_SIZE_SELECTOR, HeColorSizeSelectorClass))

#define HE_IS_COLOR_SIZE_SELECTOR(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_COLOR_SIZE_SELECTOR))

#define HE_IS_COLOR_SIZE_SELECTOR_CLASS(vtable) \
	(G_TYPE_CHECK_CLASS_TYPE ((vtable), HE_TYPE_COLOR_SIZE_SELECTOR))

#define HE_COLOR_SIZE_SELECTOR_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS ((inst), \
	HE_TYPE_COLOR_SIZE_SELECTOR, HeColorSizeSelectorClass))

typedef struct _HeColorSizeSelector              HeColorSizeSelector;
typedef struct _HeColorSizeSelectorClass         HeColorSizeSelectorClass;
typedef struct _HeColorSizeSelectorPrivate       HeColorSizeSelectorPrivate;

struct _HeColorSizeSelector
{
	GtkVBox parent_instance;

	/*< private >*/
	HeColorSizeSelectorPrivate *priv;
};

struct _HeColorSizeSelectorClass
{
	GtkVBoxClass parent_class;

	/* signals */
};

/* Construction */
GType
he_color_size_selector_get_type (void) G_GNUC_CONST;

HeColorSizeSelector *
he_color_size_selector_new      (void);

/* Data management */
gboolean
he_color_size_selector_set_size (HeColorSizeSelector *selector,
		gdouble size);

void
he_color_size_selector_get_size (HeColorSizeSelector *selector,
		gdouble *size);

gboolean
he_color_size_selector_set_color (HeColorSizeSelector *selector,
		GdkColor *color);

void
he_color_size_selector_get_color (HeColorSizeSelector *selector,
		GdkColor *color);

G_END_DECLS

#endif /* __HE_SIZE_COLOR_SELECTOR_H__ */
