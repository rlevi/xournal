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

#ifndef					__HE_COLOR_BUTTON_H__
#define					__HE_COLOR_BUTTON_H__

#include				<gtk/gtk.h>

G_BEGIN_DECLS
  #define                                         HE_TYPE_COLOR_BUTTON \
                                                  (he_color_button_get_type ())
  
  #define                                         HE_COLOR_BUTTON(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                  HE_TYPE_COLOR_BUTTON, HeColorButton))
  
  #define                                         HE_COLOR_BUTTON_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                  HE_TYPE_COLOR_BUTTON, HeColorButtonClass))
  
  #define                                         HE_IS_COLOR_BUTTON(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_COLOR_BUTTON))
  
  #define                                         HE_IS_COLOR_BUTTON_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_TYPE ((vtable), HE_TYPE_COLOR_BUTTON))
  
  #define                                         HE_COLOR_BUTTON_GET_CLASS(inst) \
                                                  (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                  HE_TYPE_COLOR_BUTTON, HeColorButtonClass))

typedef struct			_HeColorButton HeColorButton;

typedef struct			_HeColorButtonClass HeColorButtonClass;

typedef struct          _HeColorButtonPrivate HeColorButtonPrivate;

struct					_HeColorButton
{
	GtkButton parent;
	/*< private > */
    HeColorButtonPrivate *priv;
};

struct					_HeColorButtonClass
{
	GtkButtonClass parent_class;
};

GType G_GNUC_CONST
he_color_button_get_type	(void);

GtkWidget*
he_color_button_new			(void);

GtkWidget*
he_color_button_new_with_color	(const GdkColor *color);

void
he_color_button_get_color 	(HeColorButton *button,
							 GdkColor *color);

void
he_color_button_set_color	(HeColorButton *button,
							 const GdkColor *color);

gboolean
he_color_button_get_popup_shown	(HeColorButton *button);

void
he_color_button_popdown			(HeColorButton *button);

G_END_DECLS

#endif						/* __HE_COLOR_BUTTON_H__ */
