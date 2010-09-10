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

/* he-arrowed-entry.c */

/**
 * SECTION:he-arrowed-entry
 * @short_description: HildonEntry with an arrowed button to its right
 *
 * The #HildonSizeType used is HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT.
 * 
 * The #HildonEntry in this widget has focus-in/focus-out handlers automatically 
 * connected to make sure that the button gets highlighted when the focus is on the
 * HildonEntry.
 */

#include "he-arrowed-entry.h"

G_DEFINE_TYPE (HeArrowedEntry, he_arrowed_entry, GTK_TYPE_HBOX)

#define HE_ARROWED_ENTRY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
						HE_TYPE_ARROWED_ENTRY, HeArrowedEntryPrivate))

typedef struct _HeArrowedEntryPrivate HeArrowedEntryPrivate;

struct _HeArrowedEntryPrivate
{
	GtkWidget *entry;
	GtkWidget *button;
};

static gboolean he_arrowed_entry_entry_focus_in_event (const GtkWidget *widget G_GNUC_UNUSED, const GdkEventFocus *event G_GNUC_UNUSED, HeArrowedEntry *self)
{
	HeArrowedEntryPrivate *priv = HE_ARROWED_ENTRY_GET_PRIVATE (self);
	g_return_val_if_fail (priv, FALSE);

	gtk_widget_set_name (priv->button, "hildon-combobox-active-button");

	return FALSE;
}

static gboolean he_arrowed_entry_entry_focus_out_event (const GtkWidget *widget G_GNUC_UNUSED, const GdkEventFocus *event G_GNUC_UNUSED, HeArrowedEntry *self)
{
	HeArrowedEntryPrivate *priv = HE_ARROWED_ENTRY_GET_PRIVATE (self);
	g_return_val_if_fail (priv, FALSE);

	gtk_widget_set_name (priv->button, "hildon-combobox-button");

	return FALSE;
}

static void he_arrowed_entry_class_init (HeArrowedEntryClass *klass)
{
	g_type_class_add_private (klass, sizeof (HeArrowedEntryPrivate));
}

static void he_arrowed_entry_init (HeArrowedEntry *self)
{
	HeArrowedEntryPrivate *priv = HE_ARROWED_ENTRY_GET_PRIVATE (self);
	g_return_if_fail (priv);

	gtk_widget_set_name (GTK_WIDGET (self), "hildon-combobox");

	priv->entry = hildon_entry_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
	g_signal_connect (G_OBJECT (priv->entry), "focus-in-event", G_CALLBACK (he_arrowed_entry_entry_focus_in_event), self);
	g_signal_connect (G_OBJECT (priv->entry), "focus-out-event", G_CALLBACK (he_arrowed_entry_entry_focus_out_event), self);

	priv->button = hildon_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_VERTICAL);
	gtk_widget_set_name (priv->button, "hildon-combobox-button");
	gtk_widget_set_size_request (priv->button, 60, -1);

	gtk_box_pack_start (GTK_BOX (self), priv->entry, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (self), priv->button, FALSE, FALSE, 0);

	gtk_widget_show (priv->entry);
	gtk_widget_show (priv->button);
}

/**
 * he_arrowed_entry_new:
 *
 * Creates a new #HeArrowedEntry.
 *
 * Returns: a new #HildonEntry, cast as #GtkWidget
 */
GtkWidget* he_arrowed_entry_new (void)
{
	return GTK_WIDGET (g_object_new (HE_TYPE_ARROWED_ENTRY, NULL));
}

/**
 * he_arrowed_entry_get_entry:
 * @self: A #HeArrowedEntry
 *
 * Returns: The #HildonEntry assigned to the instance.
 */
GtkWidget* he_arrowed_entry_get_entry (HeArrowedEntry *self)
{
	g_return_val_if_fail (HE_IS_ARROWED_ENTRY (self), NULL);
	HeArrowedEntryPrivate *priv = HE_ARROWED_ENTRY_GET_PRIVATE (self);

	return priv->entry;
}

/**
 * he_arrowed_entry_get_button:
 * @self: A #HeArrowedEntry
 *
 * Returns: The #HildonButton assigned to the instance.
 */
GtkWidget* he_arrowed_entry_get_button (HeArrowedEntry *self)
{
	g_return_val_if_fail (HE_IS_ARROWED_ENTRY (self), NULL);
	HeArrowedEntryPrivate *priv = HE_ARROWED_ENTRY_GET_PRIVATE (self);

	return priv->button;
}
