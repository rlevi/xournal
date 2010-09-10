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

#ifndef					__HE_FONT_BUTTON_H__
#define					__HE_FONT_BUTTON_H__

#include				<gtk/gtk.h>

G_BEGIN_DECLS
  #define                                         HE_TYPE_FONT_BUTTON \
                                                  (he_font_button_get_type ())
  
  #define                                         HE_FONT_BUTTON(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                  HE_TYPE_FONT_BUTTON, HeFontButton))
  
  #define                                         HE_FONT_BUTTON_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                  HE_TYPE_FONT_BUTTON, HeFontButtonClass))
  
  #define                                         HE_IS_FONT_BUTTON(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_FONT_BUTTON))
  
  #define                                         HE_IS_FONT_BUTTON_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_TYPE ((vtable), HE_TYPE_FONT_BUTTON))
  
  #define                                         HE_FONT_BUTTON_GET_CLASS(inst) \
                                                  (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                  HE_TYPE_FONT_BUTTON, HeFontButtonClass))

typedef struct			_HeFontButton HeFontButton;

typedef struct			_HeFontButtonClass HeFontButtonClass;

typedef struct          _HeFontButtonPrivate HeFontButtonPrivate;

struct					_HeFontButton
{
	GtkButton parent;
	/*< private > */
    HeFontButtonPrivate *priv;
};

struct					_HeFontButtonClass
{
	GtkButtonClass parent_class;
};

GType G_GNUC_CONST
he_font_button_get_type	(void);

GtkWidget*
he_font_button_new			(void);

GtkWidget*
he_font_button_new_with_font	(const gchar *font_string);

gchar*
he_font_button_get_font 	(HeFontButton *button);

void
he_font_button_set_font	(HeFontButton *button,
							 const gchar *font_string);

gboolean
he_font_button_get_popup_shown	(HeFontButton *button);

void
he_font_button_popdown			(HeFontButton *button);

G_END_DECLS

#endif						/* __HE_FONT_BUTTON_H__ */
