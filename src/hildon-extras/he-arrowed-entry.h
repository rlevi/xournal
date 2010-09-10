/*
 * HeArrowedEntry: A HildonEntry with an arrow, similar to the URL Entry in Maemo 5's browser...
 *
 * This file is a part of hildon-extras
 * Copyright (C) 2010 Faheem Pervez <trippin1@gmail.com>. All rights reserved.
 * Parts Copyright (C) 2008 Nokia Corporation, all rights reserved.
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

/* he-arrowed-entry.h */

#ifndef __HE_ARROWED_ENTRY_H__
#define __HE_ARROWED_ENTRY_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>

G_BEGIN_DECLS

#define HE_TYPE_ARROWED_ENTRY (he_arrowed_entry_get_type ())

#define HE_ARROWED_ENTRY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
						HE_TYPE_ARROWED_ENTRY, HeArrowedEntry))

#define HE_ARROWED_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
						HE_TYPE_ARROWED_ENTRY, HeArrowedEntryClass))

#define HE_IS_ARROWED_ENTRY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
						HE_TYPE_ARROWED_ENTRY))

#define HE_IS_ARROWED_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
						HE_TYPE_ARROWED_ENTRY))

#define HE_ARROWED_ENTRY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
						HE_TYPE_ARROWED_ENTRY, HeArrowedEntryClass))

typedef struct _HeArrowedEntry HeArrowedEntry;
typedef struct _HeArrowedEntryClass HeArrowedEntryClass;

struct _HeArrowedEntry
{
	GtkHBox parent;
};

struct _HeArrowedEntryClass
{
	GtkHBoxClass parent_class;
};

GType he_arrowed_entry_get_type (void);

GtkWidget* he_arrowed_entry_new (void);

GtkWidget* he_arrowed_entry_get_entry (HeArrowedEntry *self);

GtkWidget* he_arrowed_entry_get_button (HeArrowedEntry *self);

G_END_DECLS

#endif /* __HE_ARROWED_ENTRY_H__ */

