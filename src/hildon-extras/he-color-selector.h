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
  
  #ifndef                                         __HE_COLOR_SELECTOR_H__
  #define                                         __HE_COLOR_SELECTOR_H__
  
  //#include                                        "hildon-touch-selector.h"
  #include <hildon/hildon-touch-selector.h>
  
  G_BEGIN_DECLS
  #define                                         HE_TYPE_COLOR_SELECTOR \
                                                  (he_color_selector_get_type ())
  
  #define                                         HE_COLOR_SELECTOR(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                  HE_TYPE_COLOR_SELECTOR, HeColorSelector))
  
  #define                                         HE_COLOR_SELECTOR_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                  HE_TYPE_COLOR_SELECTOR, HeColorSelectorClass))
  
  #define                                         HE_IS_COLOR_SELECTOR(obj) \
                                                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HE_TYPE_COLOR_SELECTOR))
  
  #define                                         HE_IS_COLOR_SELECTOR_CLASS(vtable) \
                                                  (G_TYPE_CHECK_CLASS_TYPE ((vtable), HE_TYPE_COLOR_SELECTOR))
  
  #define                                         HE_COLOR_SELECTOR_GET_CLASS(inst) \
                                                  (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                  HE_TYPE_COLOR_SELECTOR, HeColorSelectorClass))
  
  typedef struct                                  _HeColorSelector HeColorSelector;
  typedef struct                                  _HeColorSelectorClass HeColorSelectorClass;
  typedef struct                                  _HeColorSelectorPrivate HeColorSelectorPrivate;
  
  struct                                          _HeColorSelector
  {
    GtkVBox parent_instance;
  
    /*< private > */
    HeColorSelectorPrivate *priv;
  };
  
  struct                                          _HeColorSelectorClass
  {
    GtkVBoxClass parent_class;
  
    /* signals */
  };
  
  
  /* construction */
  GType
  he_color_selector_get_type                   (void) G_GNUC_CONST;
  
  GtkWidget*
  he_color_selector_new                        (void);
  
  GtkWidget*
  he_color_selector_new_with_default_color      (GdkColor *color);
  
  /* date management */
  gboolean
  he_color_selector_select_red            (HeColorSelector *selector,
                                                   guint red);
  
  gboolean
  he_color_selector_select_green             (HeColorSelector *selector,
                                                   guint green);
  
  gboolean
  he_color_selector_select_blue        (HeColorSelector *selector,
                                                   guint blue);
  
  gboolean
  he_color_selector_set_color                 (HeColorSelector *selector,
                                                   GdkColor *color);
  void
  he_color_selector_get_color                   (HeColorSelector *selector, GdkColor *color);
  
 G_END_DECLS
  
 #endif /* __HE_COLOR_SELECTOR_H__ */
