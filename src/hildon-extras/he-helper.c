/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2009 Faheem Pervez <trippin1@gmail.com>
 * Copyright (C) 2010 Thomas Perl <thp@thpinfo.com>
 *
 * Thanks to Cornelius Hald for he_helper_request_orientation():
 * http://wiki.maemo.org/Using_Fremantle_Widgets
 *
 * The tap-and-hold functions are (c) Alberto Garcia Hierro (skyhusker),
 * taken from his XChat port.
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
 * SECTION:he-helper
 * @short_description: A collection of useful utility functions.
 *
 * Set of utility functions, designed for quickly doing every day 
 * operations.
 */

#include                                        "he-helper.h"

#define                                         CURSOR_ICON_NAME "browser_cursor"
#define                                         CURSOR_ICON_SIZE 32

#ifdef ENABLE_CURSOR_FUNCS
#if !GTK_CHECK_VERSION(2, 16, 0)
static GdkCursor*
make_blank_cursor                        (GdkDisplay *display)
{
	/* Taken and heavily modified from gdkcursor-x11.c */
	GdkCursor *cursor;
	GdkScreen *screen;
	GdkPixmap *pixmap;
	GdkColor colour = { 0, 0, 0, 0 };

	screen = gdk_display_get_default_screen (display);
	g_return_val_if_fail (screen, NULL);

	pixmap = gdk_bitmap_create_from_data (gdk_screen_get_root_window (screen), "\0\0\0\0\0\0\0\0", 1, 1);  
	
	if (display->closed)
		return NULL;
	else
		cursor = gdk_cursor_new_from_pixmap (pixmap, pixmap, &colour, &colour, 1, 1);

  	g_object_unref (pixmap);

  	return cursor;
}
#endif
#endif

static void
send_gchar_mce_request                       (osso_context_t *context, const gchar *method, const gchar *arg_string)
{
    g_return_if_fail (context);
    g_return_if_fail (method);
    g_return_if_fail (arg_string);

    osso_rpc_run_system (context,
                     MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, method,
                     NULL, DBUS_TYPE_STRING, arg_string, DBUS_TYPE_INVALID);
}

/**
 * he_helper_vibrate:
 * @context: An osso_initialize()d osso_context_t.
 * @vibration_pattern: A vibration pattern from /etc/mce/mce.ini.
 * @activate: Activate pattern if TRUE; deactivate pattern if FALSE.
 *
 * Tells MCE (Mode Control Entity) to activate/deactivate a vibration pattern.
 **/
void
he_helper_vibrate                            (osso_context_t *context, const gchar *vibration_pattern, const gboolean activate)
{
    send_gchar_mce_request (context,
                     activate ? MCE_ACTIVATE_VIBRATOR_PATTERN : MCE_DEACTIVATE_VIBRATOR_PATTERN, vibration_pattern);
}

/**
 * he_helper_led:
 * @context: An osso_initialize()d osso_context_t.
 * @led_pattern: An LED pattern from /etc/mce/mce.ini. Required for both activation/deactivation.
 * @activate: Activate pattern if TRUE; deactivate pattern if FALSE.
 *
 * Tells MCE (Mode Control Entity) to activate/deactivate an LED pattern.
 **/
void
he_helper_led                                (osso_context_t *context, const gchar *led_pattern, const gboolean activate)
{
    send_gchar_mce_request (context,
                     activate ? MCE_ACTIVATE_LED_PATTERN : MCE_DEACTIVATE_LED_PATTERN, led_pattern);
}

/**
 * he_helper_accelerometer_enable:
 * @context: An osso_initialize()d osso_context_t.
 *
 * Tells MCE (Mode Control Entity) to enable the accelerometer so that you
 * can listen for the sig_device_orientation_ind signal to be notified of
 * orientation changes.
 **/
void
he_helper_accelerometer_enable         (osso_context_t *context)
{
    g_return_if_fail (context);

    osso_rpc_run_system (context,
                     MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_ACCELEROMETER_ENABLE_REQ,
                     NULL, DBUS_TYPE_INVALID);
}

/**
 * he_helper_accelerometer_disable:
 * @context: An osso_initialize()d osso_context_t.
 *
 * Tells MCE (Mode Control Entity) to disable the accelerometer.
 **/
void
he_helper_accelerometer_disable        (osso_context_t *context)
{
    g_return_if_fail (context);

    osso_rpc_run_system (context,
                     MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_ACCELEROMETER_DISABLE_REQ,
                     NULL, DBUS_TYPE_INVALID);
}

/**
 * he_helper_request_orientation:
 * @context: An osso_initialize()d osso_context_t.
 *
 * Returns the current physical orientation of the device.
 *
 * Return value: TRUE if in portrait; FALSE if otherwise.
 **/
gboolean
he_helper_request_orientation          (osso_context_t *context)
{
    osso_rpc_t ret = { 0 };
    gboolean retval = FALSE;

    g_return_val_if_fail (context, retval);

    if (osso_rpc_run_system (context,
                     MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DEVICE_ORIENTATION_GET,
                     &ret, DBUS_TYPE_INVALID) == OSSO_OK) {

        if (!g_strcmp0 (ret.value.s, MCE_ORIENTATION_PORTRAIT)) {
            retval = TRUE;
        }

        osso_rpc_free_val(&ret);
    }

    return retval;
}

#ifdef ENABLE_CURSOR_FUNCS
/**
 * he_helper_show_cursor:
 * @context: A realized GtkWidget.
 *
 * Shows a cursor on the screen, for a window.
 * Its method of getting the x and y coordinate is dodgy - please fix...
 *
 **/
void
he_helper_show_cursor                  (GtkWidget *widget)
{
	GdkDisplay *display;
	GdkScreen *screen;
	GtkIconTheme *theme;
	GdkPixbuf *pixbuf;
	gint x;
	gint y;
	GdkCursor *cursor = NULL;

	g_return_if_fail (widget);
	g_return_if_fail (GTK_WIDGET_DRAWABLE(widget));

	display = gdk_drawable_get_display (GDK_DRAWABLE(widget->window));
	g_return_if_fail (display);

	screen = gtk_widget_get_screen (widget);
	g_return_if_fail (screen);

	theme = gtk_icon_theme_get_for_screen (screen);
	g_return_if_fail (theme);

	pixbuf = gtk_icon_theme_load_icon (theme, CURSOR_ICON_NAME, CURSOR_ICON_SIZE, 0, NULL);
	if (!pixbuf)
		goto cleanup;

	x = (gdk_pixbuf_get_width (pixbuf) / 2) - 7;
	y = (gdk_pixbuf_get_height (pixbuf) / 2) - 7;

	cursor = gdk_cursor_new_from_pixbuf (display, pixbuf, x, y);
	if (!cursor)
		goto cleanup;

	gdk_window_set_cursor (widget->window, cursor);

cleanup:
	if (pixbuf)
		g_object_unref (pixbuf); 
	if (cursor)
		gdk_cursor_unref (cursor);
}

/**
 * he_helper_hide_cursor:
 * @context: A realized GtkWidget.
 *
 * Hides a cursor on the screen, for a window.
 *
 **/
void
he_helper_hide_cursor                  (GtkWidget *widget)
{
	GdkDisplay *display;
	GdkCursor *cursor = NULL;

	g_return_if_fail (widget);
	g_return_if_fail (GTK_WIDGET_DRAWABLE(widget));

	display = gdk_drawable_get_display (GDK_DRAWABLE(widget->window));
	g_return_if_fail (display);

	#if GTK_CHECK_VERSION(2, 16, 0)
	cursor = gdk_cursor_new_for_display (display, GDK_BLANK_CURSOR);
	#else
	cursor = make_blank_cursor (display);
	#endif
	if (!cursor)
		return;

	gdk_window_set_cursor (widget->window, cursor);

	gdk_cursor_unref (cursor);
}
#endif

static void
tap_and_hold_simulate_click (GtkWidget *w, gpointer btn_ptr)
{
	GdkEvent *press;
	GdkEvent *release;
	guint button;

	button = GPOINTER_TO_UINT (btn_ptr);

	press = g_object_get_data (G_OBJECT (w), "press");

	if (press) {
		/* Generate a release event for button 1 */
		release = gdk_event_copy (press);
		release->button.type = GDK_BUTTON_RELEASE;
		release->button.state = GDK_BUTTON1_MASK;
		/* Make the press event right/middle click */
		press->button.button = button;
		/* Release button 1 */
		gtk_main_do_event (release);
		/* Simulate click */
		gtk_main_do_event (press);

		/* Free the release event - the press one is
		 * freed in tap_and_hold_button_press_filter()
		 */
		gdk_event_free (release);
	}
}

static gboolean
tap_and_hold_button_press_filter (GtkWidget *w, GdkEventButton *e)
{
	GdkEvent *press;

	if (e->type == GDK_BUTTON_PRESS && e->button == 1) {
		/* Store button 1 event, so we don't need
		 * to find the pointer coordinates
		 * later
		 */
		press = g_object_get_data (G_OBJECT (w), "press");
		/* Free the previous stored one */
		if (press)
			gdk_event_free (press);
		press = gdk_event_copy ((GdkEvent*) e);
		g_object_set_data (G_OBJECT (w), "press", press);
	}

	return FALSE;
}

/**
 * he_helper_tap_and_hold_setup:
 * @widget: A #GtkWidget for which to activate tap-and-hold for.
 * @tap_and_hold_id: location to store the tap-and-hold signal handler's ID; pass NULL if the value is to be discarded
 * @button_press_event_id: location to store the button-press-event signal handler's ID; pass NULL if the value is to be discarded
 *
 * Fakes a 3rd-button press when tap-and-hold is done on @widget.
 *
 **/
void
he_helper_tap_and_hold_setup                  (GtkWidget *widget, gulong *tap_and_hold_id, gulong *button_press_event_id)
{
	gulong tmp_tap_and_hold_id;
	gulong tmp_button_press_event_id;

	g_return_if_fail (GTK_IS_WIDGET (widget));

	/* Enable tap-and-hold */
	gtk_widget_tap_and_hold_setup (widget, NULL, NULL, GTK_TAP_AND_HOLD_NONE);
	/* Setup the tap and hold handler callback */
	tmp_tap_and_hold_id = g_signal_connect (G_OBJECT (widget), "tap-and-hold", G_CALLBACK (tap_and_hold_simulate_click), GUINT_TO_POINTER (3));
	/* Setup the filter callback */
	tmp_button_press_event_id = g_signal_connect (G_OBJECT (widget), "button-press-event", G_CALLBACK (tap_and_hold_button_press_filter), NULL);

	if (tap_and_hold_id)
		*tap_and_hold_id = tmp_tap_and_hold_id;
	if (button_press_event_id)
		*button_press_event_id = tmp_button_press_event_id;
}

/**
 * he_helper_get_logical_font_desc:
 * @name: The logical font name (see he-helper.h for possible values)
 *
 * Returns a newly-allocated string that contains the string representation
 * of the Pango font description for the given logical string. This can be
 * used in the font_desc attribute of span elements in Pango markup.
 *
 * This function should be used to get the font desc for Pango markup in
 * special use cases (e.g. GtkTreeView, mixed-content GtkLabel). If you
 * want to set a logical font directly on a widget, you can use
 * #hildon_helper_set_logical_font (from Hildon 2.2) for this.
 *
 * The return value should be freed with g_free() after use.
 **/
gchar *
he_helper_get_logical_font_desc (const gchar *name)
{
    GtkSettings *settings = gtk_settings_get_default();
    GtkStyle *style = gtk_rc_get_style_by_paths(settings,
            name, NULL, G_TYPE_NONE);

    return pango_font_description_to_string(style->font_desc);
}

/**
 * he_helper_get_logical_font_color:
 * @name: The logical font color (see he-helper.h for possible values)
 *
 * Returns a newly-allocated string that contains the color value for the
 * requested logical color. This can be used in the foreground attribute of
 * span elements in Pango markup.
 *
 * This function should be used to get the font color for Pango markup in
 * special use cases (e.g. GtkTreeView, mixed-content GtkLabel). If you
 * want to set a logical color directly on a widget, you can use
 * #hildon_helper_set_logical_color (from Hildon 2.2) for this.
 *
 * The return value should be freed with g_free() after use.
 **/
gchar *
he_helper_get_logical_font_color (const gchar *name)
{
    GdkColor color;
    GtkSettings *settings = gtk_settings_get_default();
    GtkStyle *style = gtk_rc_get_style_by_paths(settings,
            "GtkButton", "osso-logical-colors", GTK_TYPE_BUTTON);

    if (gtk_style_lookup_color(style, name, &color)) {
        return gdk_color_to_string(&color);
    } else {
        return NULL;
    }
}

