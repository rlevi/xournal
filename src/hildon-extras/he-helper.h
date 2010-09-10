/*
 * This file is a part of hildon-extras
 *
 * Copyright (C) 2009 Faheem Pervez <trippin1@gmail.com>
 * Copyright (C) 2010 Thomas Perl <thp@thpinfo.com>
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

#ifndef                                         __HE_HELPER_H__
#define                                         __HE_HELPER_H__

#include                                        <glib.h>
#include                                        <X11/Xatom.h>
#include                                        <gdk/gdk.h>
#include                                        <gdk/gdkx.h>
#include                                        <gtk/gtk.h>
#include                                        <mce/dbus-names.h>
#include                                        <mce/mode-names.h>
#include                                        <dbus/dbus.h>
#include                                        <libosso.h>

//#define ENABLE_CURSOR_FUNCS

G_BEGIN_DECLS

void
he_helper_vibrate                               (osso_context_t *context, const gchar *vibration_pattern, const gboolean activate);

void
he_helper_led                                   (osso_context_t *context, const gchar *led_pattern, const gboolean activate);

void
he_helper_accelerometer_enable                  (osso_context_t *context);

void
he_helper_accelerometer_disable                 (osso_context_t *context);

gboolean
he_helper_request_orientation                   (osso_context_t *context);

#ifdef ENABLE_CURSOR_FUNCS
void
he_helper_show_cursor                           (GtkWidget *widget);

void
he_helper_hide_cursor                           (GtkWidget *widget);
#endif

void
he_helper_tap_and_hold_setup                    (GtkWidget *widget, gulong *tap_and_hold_id, gulong *button_press_event_id);

/**
 * Logical fonts
 * See the Fremantle Master Layout Guide v1.1, page 9 for details on which
 * logical font name should be used for which part of the UI.
 *
 * http://www.forum.nokia.com/info/sw.nokia.com/id/e778ba1f-2507-4672-be45-798359a3aea7/Fremantle_Master_Layout_Guide.html
 **/

#define HE_FONT_SYSTEM_FONT                  "SystemFont"
#define HE_FONT_EMP_SYSTEM_FONT              "EmpSystemFont"
#define HE_FONT_SMALL_SYSTEM_FONT            "SmallSystemFont"
#define HE_FONT_EMP_SMALL_SYSTEM_FONT        "EmpSmallSystemFont"
#define HE_FONT_LARGE_SYSTEM_FONT            "LargeSystemFont"
#define HE_FONT_X_LARGE_SYSTEM_FONT          "X-LargeSystemFont"
#define HE_FONT_XX_LARGE_SYSTEM_FONT         "XX-LargeSystemFont"
#define HE_FONT_XXX_LARGE_SYSTEM_FONT        "XXX-LargeSystemFont"
#define HE_FONT_HOME_SYSTEM_FONT             "HomeSystemFont"

gchar *
he_helper_get_logical_font_desc                 (const gchar *name);

/**
 * Logical colors
 * See the Fremantle Master Layout Guide v1.1, page 64-67 for details on which
 * logical colors should be used for which part of the UI.
 *
 * http://www.forum.nokia.com/info/sw.nokia.com/id/e778ba1f-2507-4672-be45-798359a3aea7/Fremantle_Master_Layout_Guide.html
 **/

#define HE_COLOR_DEFAULT_BACKGROUND          "DefaultBackgroundColor"
#define HE_COLOR_DEFAULT_TEXT                "DefaultTextColor"
#define HE_COLOR_SECONDARY_TEXT              "SecondaryTextColor"
#define HE_COLOR_ACTIVE_TEXT                 "ActiveTextColor"
#define HE_COLOR_SELECTION                   "SelectionColor"
#define HE_COLOR_PAINTED_TEXT                "PaintedTextColor"
#define HE_COLOR_DISABLED_TEXT               "DisabledTextColor"

#define HE_COLOR_DEFAULT_BACKGROUND_REVERSED "ReversedDefaultBackgroundColor"
#define HE_COLOR_DEFAULT_TEXT_REVERSED       "ReversedDefaultTextColor"
#define HE_COLOR_SECONDARY_TEXT_REVERSED     "ReversedSecondaryTextColor"
#define HE_COLOR_ACTIVE_TEXT_REVERSED        "ReversedActiveTextColor"
#define HE_COLOR_SELECTION_REVERSED          "ReversedSelectionColor"
#define HE_COLOR_PAINTED_TEXT_REVERSED       "ReversedPaintedTextColor"
#define HE_COLOR_DISABLED_TEXT_REVERSED      "ReversedDisabledTextColor"

#define HE_COLOR_CONTENT_BACKGROUND          "ContentBackgroundColor"
#define HE_COLOR_CONTENT_FRAME               "ContentFrameColor"
#define HE_COLOR_CONTENT_SELECTION           "ContentSelectionColor"

#define HE_COLOR_TITLE_TEXT                  "TitleTextColor"
#define HE_COLOR_BUTTON_TEXT                 "ButtonTextColor"
#define HE_COLOR_BUTTON_TEXT_PRESSED         "ButtonTextPressedColor"
#define HE_COLOR_BUTTON_TEXT_DISABLED        "ButtonTextDisabledColor"

#define HE_COLOR_ACCENT_1                    "AccentColor1"
#define HE_COLOR_ACCENT_2                    "AccentColor2"
#define HE_COLOR_ACCENT_3                    "AccentColor3"

#define HE_COLOR_ATTENTION                   "AttentionColor"

#define HE_COLOR_NOTIFICATION_BACKGROUND     "NotificationBackgroundColor"
#define HE_COLOR_NOTIFIACTION_TEXT           "NotificationTextColor"
#define HE_COLOR_NOTIFICATION_SECONDARY_TEXT "NotificationSecondaryTextColor"

gchar *
he_helper_get_logical_font_color                (const gchar *name);

G_END_DECLS

#endif                                          /* __HE_HELPER_H__ */


