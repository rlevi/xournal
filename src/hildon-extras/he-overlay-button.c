/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2009 Cornelius Hald <hald@icandy.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 */


/**
 * SECTION:he-overlay-button
 * @short_description: A semi-transparent button to leave fullscreen mode.
 *
 * #HeOverlayButton is a semi transparent button which is automatically
 * displayed whenever it's parent window enters fullscreen state.
 * It's always displayed in the lower right corner of the parent window.
 *
 * The button is automatically hidden after 5 seconds of mouse click
 * inactivity. If the user clicks the parent window the button is shown
 * for another 5 second.
 *
 * If the user clicks the button, the "clicked" signal is emitted. If you did
 * not provide a signal handler for the "clicked" signal, then the default
 * handler will call gtk_window_unfullscreen() on the parent window.
 * If you provide a signal handler, the default handler will not be called
 * at all.
 *
 * So, if your application has just one window. It will be enough, if you create
 * an instance of HeOverlayButton with this window as parent. Now if your
 * window switches to fullscreen the HeOverlayButton is automatically shown
 * and can be used to leave fullscreen mode. In this case you don't have to provide
 * a signal handler and you don't have to take care of the buttons destruction.
 */

#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <string.h>
#include "he-overlay-button.h"

#define OVERLAY_BUTTON_WIDTH          80
#define OVERLAY_BUTTON_HEIGHT         70
#define OVERLAY_BUTTON_HIDE_DELAY     5000
#define OVERLAY_BUTTON_CORNER_RADIUS  20
#define OVERLAY_BUTTON_ICON           "general_fullsize"
#define OVERLAY_BUTTON_ICON_SIZE      48

typedef struct				_HeOverlayButtonPrivate HeOverlayButtonPrivate;

struct _HeOverlayButtonPrivate
{
        gboolean     release_event;
        guint32      last_event_time;

        guint        button_press_signal_id;
        guint        button_release_signal_id;

        gulong       button_press_hook_id;
        gulong       button_release_hook_id;

        gboolean     act_on_state_change;

        GtkWidget   *overlay;
};

#define						HE_OVERLAY_BUTTON_GET_PRIVATE(object) \
							(G_TYPE_INSTANCE_GET_PRIVATE((object), \
							HE_TYPE_OVERLAY_BUTTON, HeOverlayButtonPrivate))

G_DEFINE_TYPE(HeOverlayButton, he_overlay_button, G_TYPE_OBJECT)


/**
 * Hides the overlay button.
 */
static void
overlay_button_hide (HeOverlayButton *self)
{
    g_return_if_fail (HE_IS_OVERLAY_BUTTON (self));

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    /* Reset timer */
    g_source_remove_by_user_data ((gpointer) self);

    if (priv->overlay != NULL && GTK_IS_WIDGET (priv->overlay)) {
    	gtk_widget_hide (priv->overlay);
    }
}


/**
 * Changes the position of the overlay button.
 */
static void
overlay_button_set_position (HeOverlayButton *self, gint x, gint y)
{
	GtkWidget *parent = GTK_WIDGET (self->parent_window);
	GtkWidget *overlay = NULL;

	HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
	g_return_if_fail (priv != NULL);

	overlay = GTK_WIDGET (priv->overlay);

	/* For some reason I have to call hide/show to make it appear at the new position */
	gtk_widget_hide (overlay);
	gtk_window_move (GTK_WINDOW (overlay), x, y);
	gtk_widget_show (overlay);
}


/**
 * Changes the position of the overlay button.
 */
static void
overlay_button_set_predefined_position (HeOverlayButton *self, gint position)
{
	GtkWidget *parent = GTK_WIDGET (self->parent_window);
	GtkWidget *overlay = NULL;
	gint x, y;

	HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
	g_return_if_fail (priv != NULL);

	overlay = GTK_WIDGET (priv->overlay);

	switch (position) {
		case TOP_HORIZ_CENTERED:
			x = (parent->allocation.width - overlay->allocation.width) / 2;
			y = 0;
			break;
		case BOTTOM_HORIZ_CENTERED:
			x = (parent->allocation.width - overlay->allocation.width) / 2;
			y = parent->allocation.height - overlay->allocation.height;
			break;
		case LEFT_VERTICAL_CENTERED:
			x = 0;
			y = (parent->allocation.height - overlay->allocation.height) / 2;
			break;
		case RIGHT_VERTICAL_CENTERED:
			x = parent->allocation.width - overlay->allocation.width;
			y = (parent->allocation.height - overlay->allocation.height) / 2;
			break;
		default:
			x = (parent->allocation.width - overlay->allocation.width) / 2;
			y = 0;
			break;
	}

	/* For some reason I have to call hide/show to make it appear at the new position */
	gtk_widget_hide (overlay);
	gtk_window_move (GTK_WINDOW (overlay), x, y);
	gtk_widget_show (overlay);
}


/**
 * Everytime the timer runs out, we hide the overlay button.
 */
static gboolean
overlay_button_on_hide_timer (gpointer data)
{
    g_return_val_if_fail (data != NULL, FALSE);
    overlay_button_hide (HE_OVERLAY_BUTTON (data));
    return FALSE;
}


/**
 * Shows the overlay button.
 */
static void
overlay_button_show (HeOverlayButton *self)
{
    g_return_if_fail (self != NULL);

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    g_return_if_fail (GTK_IS_WIDGET (priv->overlay));

    /* Stop return button hide timeout */
    g_source_remove_by_user_data ((gpointer) self);

    /* Only show overlay if we come here through a button release event, not a button press event */
    if (priv->release_event) {

    	overlay_button_set_position (self);
    	gtk_widget_show (priv->overlay);

        /* Set the return button hide timeout */
        g_timeout_add (OVERLAY_BUTTON_HIDE_DELAY,
        		overlay_button_on_hide_timer, (gpointer) self);
    }
}


/**
 * This hook function is called for each mouse button press or
 * button release on the parent window.
 */
static gboolean
overlay_button_input_activity_hook (GSignalInvocationHint *ihint G_GNUC_UNUSED,
                                       guint n_param_values,
                                       const GValue *param_values,
                                       gpointer data)
{
    HeOverlayButton *self = HE_OVERLAY_BUTTON (data);
    g_return_val_if_fail (self, FALSE);

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_val_if_fail (priv != NULL, FALSE);

    GdkEventAny *event = NULL;

    if (n_param_values >= 2)
        event = (GdkEventAny*) g_value_peek_pointer (&(param_values[1]));

    g_return_val_if_fail (event, TRUE);

    guint32 time = 0;
    switch (event->type) {
        case GDK_BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            time = ((GdkEventButton*) event)->time;
            break;
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
            time = ((GdkEventKey*) event)->time;
            break;
        default:
            /* Filter out unexpected messages */
            return TRUE;
    }

    /* We're likely to get events multiple times as they're propagated, so
       filter out events that we've already seen. */
    if (time == priv->last_event_time) {
        return TRUE;
    }
    priv->last_event_time = time;

    if (event && (event->type == GDK_BUTTON_PRESS)) {
        priv->release_event = FALSE;
    } else {
        priv->release_event = TRUE;
    }

    overlay_button_show (self);

    return TRUE;
}


/**
 * This function makes the overlay screen button visible and hooks mouse and
 * key event signal emissions. The button is hidden after some time and
 * is reshown when ever one of the signal hooks are activated.
 *
 * Note: The button may be shown automatically by itself
 * if you have not set the act_on_state_change property to
 * FALSE.
 *
 * @param self A HeOverlayButton instance.
 */
void
he_overlay_button_enable (HeOverlayButton *self)
{
    g_return_if_fail(HE_IS_OVERLAY_BUTTON(self));

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    if (priv->button_press_hook_id == 0) {
        priv->button_press_signal_id =
        	g_signal_lookup ("button-press-event", GTK_TYPE_WIDGET);
        priv->button_press_hook_id =
            g_signal_add_emission_hook (priv->button_press_signal_id, 0,
            		overlay_button_input_activity_hook,
            		(gpointer) self, NULL);
    }

    if (priv->button_release_hook_id == 0) {
        priv->button_release_signal_id =
        	g_signal_lookup ("button-release-event", GTK_TYPE_WIDGET);
        priv->button_release_hook_id =
            g_signal_add_emission_hook (priv->button_release_signal_id, 0,
            		overlay_button_input_activity_hook,
            		(gpointer) self, NULL);
    }

    overlay_button_show(self);
}


/**
 * Hides the overlay button and releases mouse and
 * key event signal emission hooks.
 *
 * Note: The button may be hidden automatically by itself
 * if you have not set the act_on_state_change property to
 * FALSE.
 *
 * @param self A HeOverlayButton instance.
 */
void
he_overlay_button_disable (HeOverlayButton *self)
{
    g_return_if_fail (HE_IS_OVERLAY_BUTTON (self));

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    overlay_button_hide (self);

    if (priv->button_release_hook_id > 0) {
        g_signal_remove_emission_hook (priv->button_release_signal_id,
                                       priv->button_release_hook_id);
        priv->button_release_hook_id = 0;
    }

    if (priv->button_press_hook_id > 0) {
        g_signal_remove_emission_hook (priv->button_press_signal_id,
                                       priv->button_press_hook_id);
        priv->button_press_hook_id = 0;
    }
}


/**
 * Everytime the button is clicked, be emmit the "clicked" signal.
 */
static gboolean
overlay_button_on_clicked (GtkWidget *widget, GdkEventButton *event G_GNUC_UNUSED, gpointer data)
{
	HeOverlayButton *self = HE_OVERLAY_BUTTON (data);
	g_signal_emit_by_name (self, "clicked");

	return TRUE;
}


/**
 * Creates a rectangle with a rounded upper left corner.
 */
static void
overlay_button_create_rectangle (cairo_t *ctx, double x, double y, double w, double h, double r)
{
	cairo_move_to(ctx, x+r, y);
	cairo_line_to(ctx, x+w, y);
	cairo_line_to(ctx, x+w, y+h);
	cairo_line_to(ctx, x,   y+h);
	cairo_line_to(ctx, x,   y+r);

	/* TODO: either curve all the angles or none */
	/* Left upper corner is rounded */
	// cairo_curve_to(ctx, x, y, x, y, x+r, y);

}


/**
 * Does the actuall drawing of the semi transparent button graphic.
 */
static gboolean
overlay_button_on_expose_event (GtkWidget *widget, GdkEventExpose *event G_GNUC_UNUSED, gpointer data)
{
    cairo_t *ctx;
    GdkPixbuf *pixbuf = GDK_PIXBUF (data);

    /* Create context */
    ctx = gdk_cairo_create (widget->window);

    /* Clear surface */
    cairo_set_operator (ctx, CAIRO_OPERATOR_CLEAR);
    cairo_paint (ctx);

    /* Add rectangle */
    cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba (ctx, 0, 0, 0, 0.60);
    overlay_button_create_rectangle (ctx, 0, 0, OVERLAY_BUTTON_WIDTH, OVERLAY_BUTTON_HEIGHT, OVERLAY_BUTTON_CORNER_RADIUS);
    cairo_fill (ctx);

    /* Add icon */
    gdk_cairo_set_source_pixbuf (ctx, pixbuf, 15, 10);
    cairo_paint (ctx);

    /* Destroy context */
    cairo_destroy (ctx);
    return TRUE;
}


/**
 * Creates the semi transparent button graphic.
 */
static GtkWidget*
overlay_button_create_gfx (HeOverlayButton *self, const gchar *overlay_button_icon, GtkWidget *widget)
{
    GdkPixbuf *pixbuf;
    GtkWidget *img = widget;
 
    g_return_val_if_fail(HE_IS_OVERLAY_BUTTON(self), NULL);

    if (!img) {
    	pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), overlay_button_icon, OVERLAY_BUTTON_ICON_SIZE, 0, NULL);
    	img = gtk_image_new_from_pixbuf (pixbuf);
    } else {
	pixbuf = gtk_image_get_pixbuf ((GdkImage *) img);
    }

    gtk_widget_show (img);
    /* TODO: should we really unref it ? */
    g_object_unref (pixbuf);
    g_signal_connect (img, "expose_event", G_CALLBACK (overlay_button_on_expose_event), pixbuf);

    GtkWidget *box = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(box), FALSE);
    gtk_widget_show (box);
    gtk_container_add (GTK_CONTAINER(box), img);
    g_signal_connect (box, "button-release-event", G_CALLBACK (overlay_button_on_clicked), self);

    GtkWidget *overlay = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_window_set_decorated (GTK_WINDOW (overlay), FALSE);
    gtk_widget_set_size_request (overlay, OVERLAY_BUTTON_WIDTH, OVERLAY_BUTTON_HEIGHT);
    gtk_window_set_resizable (GTK_WINDOW (overlay), FALSE);
    gtk_window_set_transient_for (GTK_WINDOW (overlay), self->parent_window);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (overlay), TRUE);
    gtk_container_add (GTK_CONTAINER (overlay), box);

    GdkScreen *screen = gtk_widget_get_screen (overlay);
    gtk_widget_set_colormap (overlay, gdk_screen_get_rgba_colormap (screen));

    gtk_widget_realize (overlay);

    return overlay;
}

/**
 * Called when the parent_window's is on the screen/not on the screen.
 * Only called if parent_window is a HildonWindow (or derived from it).
 *
 * We check if the window is on the screen or not on the screen.
 * If it is, and the window is in fullscreen mode, we enable the fullscreen button. If not, we disable the button.
 */
static void
overlay_button_on_is_topmost_changed (GObject *object G_GNUC_UNUSED,
		                                 GParamSpec *property G_GNUC_UNUSED,
		                                 gpointer data)
{
	HeOverlayButton *self = HE_OVERLAY_BUTTON (data);

	HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
	g_return_if_fail (priv != NULL);

	/* Only run if the "act-on-state-change" property is TRUE. */
	if (!priv->act_on_state_change) {
		return;
	}

	if (hildon_window_get_is_topmost (HILDON_WINDOW(self->parent_window))) {
		if (gdk_window_get_state (GTK_WIDGET (self->parent_window)->window) & GDK_WINDOW_STATE_OVERLAY) {
			he_overlay_button_enable (self);
		}
	}
	else {
		he_overlay_button_disable (self);
	}
}

/**
 * Called, whenever the state of the parents window's GdkWindow changes.
 * We check if the new state is fullscreen or non-fullscreen.
 * If it is fullscreen, we enable the fullscreen button. If not, not.
 */
static gboolean
overlay_button_on_window_state_changed (GtkWidget *widget G_GNUC_UNUSED,
		                                   GdkEventWindowState *event,
		                                   gpointer data)
{
	HeOverlayButton *self = HE_OVERLAY_BUTTON (data);

	HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
	g_return_val_if_fail (priv != NULL, FALSE);

	/* Only run if the "act-on-state-change" property is TRUE. */
	if (!priv->act_on_state_change) {
		return FALSE;
	}

	/* Only run if this window is topmost. */
	if (HILDON_IS_WINDOW (self->parent_window)) {
		if (!hildon_window_get_is_topmost (HILDON_WINDOW(self->parent_window))) {
			return FALSE;
		}
	}

	if (event->changed_mask & GDK_WINDOW_STATE_OVERLAY) {

		if (event->new_window_state == GDK_WINDOW_STATE_OVERLAY) {
			he_overlay_button_enable (self);
		} else {
			he_overlay_button_disable (self);
		}

	}

	return FALSE;
}


/**
 * Destroys the fullscreen button and disconnects itself from the parent window.
 */
static void
overlay_button_destroy (GtkWidget *parent_window G_GNUC_UNUSED, HeOverlayButton *self)
{
	g_return_if_fail (self != NULL);

	HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
	g_return_if_fail (priv != NULL);

	if (self->parent_window != NULL) {
		g_signal_handlers_disconnect_by_func (self->parent_window, overlay_button_destroy, self);
		g_signal_handlers_disconnect_by_func (self->parent_window, overlay_button_on_window_state_changed, self);
		if (HILDON_IS_WINDOW(parent_window)) {
			g_signal_handlers_disconnect_by_func (self->parent_window, overlay_button_on_is_topmost_changed, self);
		}
	}

	he_overlay_button_disable (self);

	if (priv->overlay != NULL && GTK_IS_WIDGET(priv->overlay)) {
		gtk_widget_destroy (GTK_WIDGET(priv->overlay));
		priv->overlay = NULL;
	}
}


/**
 * Called when the size allocation of the parent window changes.
 * We change the position of the fullscreen button to always be in
 * the lower right corner.
 */
static void
overlay_button_on_parent_size_changed (GtkWidget     *widget,
                                          GtkAllocation *allocation,
                                          gpointer       user_data)
{
    g_return_if_fail (widget != NULL);
    g_return_if_fail (allocation != NULL);

    HeOverlayButton *self = HE_OVERLAY_BUTTON(user_data);
    g_return_if_fail (self != NULL);

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    GtkWidget *ui_win = GTK_WIDGET(priv->overlay);
    g_return_if_fail (ui_win != NULL);

    if (gdk_window_is_visible(priv->overlay->window)) {
    	overlay_button_set_position(self);
    }
}


/**
 * Default handler for the "clicked" signal. If no user handler is
 * defined we call gtk_window_unfullscreen() on the parent window.
 * Otherwise only the user handler is executed.
 */
static void
overlay_button_clicked_default_handler (HeOverlayButton *self)
{
	guint signal_id = g_signal_lookup ("clicked", HE_TYPE_OVERLAY_BUTTON);
	gulong handler_id = g_signal_handler_find (self, G_SIGNAL_MATCH_ID, signal_id, 0, NULL, NULL, NULL);

	/* Run only, if no signal handler is connected */
	if (handler_id == 0) {
		GtkWindow *window = he_overlay_button_get_window (self);
		gtk_window_unfullscreen (window);
	}
}

/**
 * Create new full screen button instance.
 * This function attaches the full screen button to the given parent window.
 * By default, the button automatically becomes visible if the parent window
 * changes to fullscreen and vice versa. Change the "act-on-state-change" 
 * property to modify this behaviour.
 *
 * If you destroy the parent window, this HeOverlayButton instance get
 * destroyed as well.
 *
 * Pass it a HildonWindow (or one of its deriatives) to ensure the widget disables/
 * enables itself on focus-out/focus-in respectively. 
 *
 * @param parent_window A GtkWindow instance.
 * @return New HeOverlayButton instance.
 */
HeOverlayButton *
he_overlay_button_new (GtkWindow *parent_window)
{
	g_return_val_if_fail (parent_window != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WINDOW (parent_window), NULL);

    HeOverlayButton *self = g_object_new (HE_TYPE_OVERLAY_BUTTON, NULL);

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_val_if_fail (priv != NULL, NULL);

    self->parent_window = parent_window;
    priv->overlay = overlay_button_create_gfx (self, "generic_fullscreen", NULL);

    g_signal_connect (parent_window, "destroy",
    		G_CALLBACK(overlay_button_destroy), self);

    g_signal_connect (parent_window, "window-state-event",
    		G_CALLBACK(overlay_button_on_window_state_changed), self);

    g_signal_connect_after (parent_window, "size-allocate",
    		G_CALLBACK(overlay_button_on_parent_size_changed), self);

    if (HILDON_IS_WINDOW(parent_window)) {
        g_signal_connect (parent_window, "notify::is-topmost",
            G_CALLBACK(overlay_button_on_is_topmost_changed), self);
    }

    return self;
}

/**
 * Returns the GtkWidget displaying the actual overlaid button.
 *
 * @param self An instance of HeOverlayButton
 * @return The GtkWidget of the overlaid button. This widget belongs to HeOverlayButton and must not be destroyed or modified.
 */
GtkWidget *
he_overlay_button_get_overlay (HeOverlayButton *self)
{
    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_val_if_fail (priv != NULL, NULL);

    return priv->overlay;
}

/**
 * Returns the GtkWindow that this HeOverlayButton
 * is attached to.
 *
 * @param self An instance of HeOverlayButton
 * @return The GtkWindow to which this button is attached to
 */
GtkWindow *
he_overlay_button_get_window (HeOverlayButton *self)
{
	return self->parent_window;
}


/*
 * GObject stuff
 */

enum {
	CLICKED,
	LAST_SIGNAL
};

enum {
	TOP_HORIZ_CENTERED,
	BOTTOM_HORIZ_CENTERED,
	LEFT_VERTICAL_CENTERED,
	RIGHT_VERTICAL_CENTERED
};

static guint signals[LAST_SIGNAL] = {0};

enum {
    PROP_0,
    PROP_ACT_ON_STATE_CHANGE
};

static void
overlay_button_get_property (GObject *object, 
                                guint property_id,
                                GValue *value, 
                                GParamSpec * pspec)
{
    HeOverlayButton *self = HE_OVERLAY_BUTTON (object);
    g_return_if_fail (self);

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    switch (property_id) {
        case PROP_ACT_ON_STATE_CHANGE:
            g_value_set_boolean (value, priv->act_on_state_change);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
overlay_button_set_property (GObject *object, 
                                guint property_id,
                                const GValue *value, 
                                GParamSpec * pspec)
{
    HeOverlayButton *self = HE_OVERLAY_BUTTON (object);
    g_return_if_fail (self);

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    switch (property_id) {
        case PROP_ACT_ON_STATE_CHANGE:
            priv->act_on_state_change = g_value_get_boolean (value);
            g_object_notify (G_OBJECT (object), "act-on-state-change");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}


static void
he_overlay_button_class_init (HeOverlayButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = overlay_button_get_property;
	object_class->set_property = overlay_button_set_property;

	klass->clicked = overlay_button_clicked_default_handler;

	/**
	 * HeOverlayButton::clicked
	 *
	 * Emitted when the #HeOverlayButton was clicked by the user.
	 */
	signals[CLICKED] =
		g_signal_new(
				"clicked",
				HE_TYPE_OVERLAY_BUTTON,
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(HeOverlayButtonClass, clicked),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0);

	g_object_class_install_property(
				object_class, PROP_ACT_ON_STATE_CHANGE,
				g_param_spec_boolean ("act-on-state-change",
				"Act on window state changes",
				"Whether to automatically enable/disable the button "
				"when its parent window fullscreens/unfullscreens "
				"itself.",
				TRUE,
				G_PARAM_READWRITE));

	g_type_class_add_private (klass, sizeof (HeOverlayButtonPrivate));
}


static void
he_overlay_button_init (HeOverlayButton *self)
{
    g_return_if_fail (self != NULL);

    HeOverlayButtonPrivate *priv = HE_OVERLAY_BUTTON_GET_PRIVATE (self);
    g_return_if_fail (priv != NULL);

    memset (priv, 0, sizeof (HeOverlayButtonPrivate));

    self->parent_window = NULL;
    priv->overlay = NULL;
    priv->release_event = TRUE;
    priv->last_event_time = 0;

    priv->act_on_state_change = TRUE;

    priv->button_press_signal_id = 0;
    priv->button_release_signal_id = 0;

    priv->button_press_hook_id = 0;
    priv->button_release_hook_id = 0;
}

