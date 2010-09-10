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
  
  #ifndef                                         __HE_FONT_SELECTOR_H__
  #define                                         __HE_FONT_SELECTOR_H__
  
  //#include                                        "hildon-touch-selector.h"
  #include <hildon/hildon-touch-selector.h>
  
  G_BEGIN_DECLS
  #define                                         HE_TYPE_FONT_SELECTOR \
                                                  (he_font_selector_get_type ())
  
  #define                                         HE_FONT_SELECTOR(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                  HE_TYPE_FONT_SELECTOR, HeFontSelector))
  
  #define                                         HE_FONT_SELECTOR_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                  HE_TYPE_FONT_SELECTOR, HeFontSelectorClass))
  
  #define                                         HE_IS_FONT_SELECTOR(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_FONT_SELECTOR))
  
  #define                                         HE_IS_FONT_SELECTOR_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_TYPE ((vtable), HE_TYPE_FONT_SELECTOR))
  
  #define                                         HE_FONT_SELECTOR_GET_CLASS(inst) \
                                                  (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                  HE_TYPE_FONT_SELECTOR, HeFontSelectorClass))
  
  typedef struct                                  _HeFontSelector HeFontSelector;
  typedef struct                                  _HeFontSelectorClass HeFontSelectorClass;
  typedef struct                                  _HeFontSelectorPrivate HeFontSelectorPrivate;
  
  struct                                          _HeFontSelector
  {
    GtkVBox parent_instance;
  
    /*< private > */
    HeFontSelectorPrivate *priv;
  };
  
  struct                                          _HeFontSelectorClass
  {
    GtkVBoxClass parent_class;
  
    /* signals */
  };
  
  
  /* construction */
  GType
  he_font_selector_get_type                   (void) G_GNUC_CONST;
  
  GtkWidget*
  he_font_selector_new                        (void);
  
  GtkWidget*
  he_font_selector_new_with_default_font        (gchar *font_string);
  
  /* date management */
  gboolean
  he_font_selector_select_family            (HeFontSelector *selector,
                                                   gchar *family);

  gboolean
  he_font_selector_select_size        (HeFontSelector *selector,
                                                   guint size);
  
  gboolean
  he_font_selector_set_font                 (HeFontSelector *selector,
                                                   gchar *font_string);
  gchar *
  he_font_selector_get_font                   (HeFontSelector *selector);
  
 G_END_DECLS
  
 #endif /* __HE_FONT_SELECTOR_H__ */
