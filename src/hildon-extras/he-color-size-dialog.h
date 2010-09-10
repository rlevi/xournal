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

#ifndef					__HE_COLOR_SIZE_DIALOG_H__
#define					__HE_COLOR_SIZE_DIALOG_H__

#include				<gtk/gtk.h>

G_BEGIN_DECLS
  #define                                         HE_TYPE_COLOR_SIZE_DIALOG \
                                                  (he_color_size_dialog_get_type ())
  
  #define                                         HE_COLOR_SIZE_DIALOG(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                  HE_TYPE_COLOR_SIZE_DIALOG, HeColorSizeDialog))
  
  #define                                         HE_COLOR_SIZE_DIALOG_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                  HE_TYPE_COLOR_SIZE_DIALOG, HeColorSizeDialogClass))
  
  #define                                         HE_IS_COLOR_SIZE_DIALOG(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_COLOR_SIZE_DIALOG))
  
  #define                                         HE_IS_COLOR_SIZE_DIALOG_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_TYPE ((vtable), HE_TYPE_COLOR_SIZE_DIALOG))
  
  #define                                         HE_COLOR_SIZE_DIALOG_GET_CLASS(inst) \
                                                  (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                  HE_TYPE_COLOR_SIZE_DIALOG, HeColorSizeDialogClass))

typedef struct			_HeColorSizeDialog HeColorSizeDialog;

typedef struct			_HeColorSizeDialogClass HeColorSizeDialogClass;

typedef struct          _HeColorSizeDialogPrivate HeColorSizeDialogPrivate;

struct					_HeColorSizeDialog
{
	GtkDialog parent;
	/*< private > */
    HeColorSizeDialogPrivate *priv;
};

struct					_HeColorSizeDialogClass
{
	GtkDialogClass parent_class;
};

GType G_GNUC_CONST
he_color_size_dialog_get_type	(void);

GtkWidget*
he_color_size_dialog_new			(void);

void
he_color_size_dialog_get_color 	(HeColorSizeDialog *dialog,
							 GdkColor *color);

void
he_color_size_dialog_set_color	(HeColorSizeDialog *dialog,
							 GdkColor *color);

G_END_DECLS

#endif						/* __HE_COLOR_SIZE_DIALOG_H__ */
