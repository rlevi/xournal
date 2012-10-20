#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <dbus/dbus.h>
#include <hildon/hildon-program.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-button.h>
#include <hildon/hildon-pannable-area.h>
#include <hildon/hildon-color-chooser.h>
#include <hildon/hildon-color-chooser-dialog.h>
#include <hildon/hildon-color-button.h>
#include <libosso.h>
#include <libintl.h>
#define _(String) gettext (String)

#include "xournal.h"
#include "xo-callbacks.h"
#include "xo-hildon_ui.h"
#include "xo-support.h"

extern char *theme_name;
extern GtkUIManager *ui_manager;
extern GtkWidget *winMain;
extern GtkWidget *hildon_cur_toolbar_landscape,
	*hildon_cur_toolbar_portrait,
	*switch_to_toolbar;
extern GtkWidget *hildon_toolbars[HILDON_NUM_TOOLBARS];
extern GtkWidget *hildon_undo[HILDON_NUM_TOOLBARS];
extern GtkWidget *hildon_redo[HILDON_NUM_TOOLBARS];

gboolean hildon_cur_toolbar_visible = TRUE;

char *hildonToolbars[HILDON_NUM_TOOLBARS] = {
	"HildonToolBarNoteLandscape",
	"HildonToolBarNotePortrait",
	"HildonToolBarPdfLandscape",
	"HildonToolBarPdfPortrait",
	"HildonToolBarDrawLandscape",
	"HildonToolBarDrawPortrait",
	"HildonToolBarHandLandscape",
	"HildonToolBarHandPortrait",
	"HildonToolBarPenLandscape",
	"HildonToolBarPenPortrait",
	"HildonToolBarPenColorLandscape",
	"HildonToolBarPenColorPortrait"
};

GtkWidget *tools_view, *settings_view, *journal_view;
//extern gint two_half_button_click, hildon_button_pressed, hildon_button_released, hildon_timer_running_already;

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

static void
register_hildon_stock_icons (void)
{
  GtkIconFactory *icon_factory;
  GtkIconSet *icon_set;
  GtkIconSource *icon_source;
  gint i, n_stock_icons;

  icon_factory = gtk_icon_factory_new ();
  n_stock_icons = G_N_ELEMENTS(hildon_stock_icons);

  for (i=0;i<n_stock_icons;i++) {
	  char *filename=calloc(1024,1);
	  GtkIconSize *sizes = NULL;
	  gint nsize;
	  gint j;

	  icon_set = gtk_icon_set_new ();
	  icon_source = gtk_icon_source_new ();
	  snprintf (filename, 1024, hildon_stock_icons[i].filename, theme_name);
	  gtk_icon_source_set_filename (icon_source, filename);
	  gtk_icon_set_add_source (icon_set, icon_source);
	  gtk_icon_factory_add (icon_factory, hildon_stock_icons[i].stock_id, icon_set);
	  gtk_icon_set_unref (icon_set);
	  free(filename);
  }

  gtk_icon_factory_add_default (icon_factory);

  g_object_unref (icon_factory);
}

static GtkWidget *add_item (HildonAppMenu *menu, const gchar *title,
		GCallback callback, gint show, gpointer user_data, GtkWidget *button, gint filter)
{
	static GtkWidget *first = NULL;

	if (filter) {
		if (first != NULL) {
			button = (button ? button : hildon_gtk_radio_button_new_from_widget (
					HILDON_SIZE_AUTO,
					GTK_RADIO_BUTTON (first)));
		} else {
			button = (button ? button: hildon_gtk_radio_button_new (
				HILDON_SIZE_AUTO,
				NULL));
			first = button;
		}

		gtk_button_set_label (GTK_BUTTON (button), title);
	} else {
		button = (button ? button : hildon_button_new_with_text (
			HILDON_SIZE_AUTO,
			HILDON_BUTTON_ARRANGEMENT_VERTICAL,
			title,
			NULL));
	}

	g_signal_connect_after (button, "clicked",
			G_CALLBACK (callback),
			user_data);
		
	if (show) {
		gtk_widget_show (button);
	} else {
		gtk_widget_hide (button);
	}

	if (filter) {
		hildon_app_menu_add_filter (menu, GTK_BUTTON(button));
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);
	} else {
		hildon_app_menu_append (menu, GTK_BUTTON(button));
	}

	return button;
}

void add_menu (HildonAppMenu *menu, XournalHildonMenu *menu_items, int num_items, gpointer user_data) {
	int i=0;

	for (i=0; i<num_items;i++) {
		menu_items[i].widget = add_item (menu,
				menu_items[i].title,
				menu_items[i].callback,
				menu_items[i].show,
				user_data,
			       	menu_items[i].widget,
				0);
	}
}

void add_filter (HildonAppMenu *menu, XournalHildonMenu *menu_items, int num_items, gpointer user_data) {
	int i=0;

	for (i=0; i<num_items;i++) {
		menu_items[i].widget = add_item (menu,
				menu_items[i].title,
				menu_items[i].callback,
				menu_items[i].show,
				user_data, 
				menu_items[i].widget,
				1);
	}
}

HildonAppMenu *hildon_create_menu (gpointer user_data)
{
	HildonAppMenu *app_menu = HILDON_APP_MENU(hildon_app_menu_new ());

	add_menu (app_menu, fremantle_menu_file_entries, 
		sizeof (fremantle_menu_file_entries)/sizeof(XournalHildonMenu),
		user_data);
	add_menu (app_menu, fremantle_menu_tools_entries,
		sizeof (fremantle_menu_tools_entries)/sizeof(XournalHildonMenu),
		user_data);
	add_filter (app_menu, fremantle_menu_filters,
		sizeof (fremantle_menu_filters)/sizeof(XournalHildonMenu),
		user_data);

	return app_menu;
}

void hildon_pen_activate (GtkWidget *item, gpointer user_data)
{
}

void hildon_eraser_activate (GtkWidget *item, gpointer user_data)
{
}

void hildon_highlighter_activate (GtkWidget *item, gpointer user_data)
{
}

void hildon_zoom_activate (GtkWidget *item, gpointer user_data)
{
}

void hildon_layer_activate (GtkWidget *item, gpointer user_data)
{
}

void hildon_page_activate (GtkWidget *item, gpointer user_data)
{
}

void hildon_paper_activate (GtkWidget *item, gpointer user_data)
{
}

void hildon_toolbar_refresh_color (GtkWidget *item, gpointer user_data)
{
	gint toolbar = (gint) user_data;
	gchar widget_path[200];
	GtkWidget *widget;

	switch (ui.cur_brush->color_no) {
		case COLOR_BLACK:
    			sprintf (widget_path, "/%s/Black", hildonToolbars[toolbar]);
			widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
			break;
		case COLOR_BLUE:
    			sprintf (widget_path, "/%s/Blue", hildonToolbars[toolbar]);
			widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
			break;
		case COLOR_RED:
    			sprintf (widget_path, "/%s/Red", hildonToolbars[toolbar]);
			widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
			break;
		case COLOR_GREEN:
    			sprintf (widget_path, "/%s/Green", hildonToolbars[toolbar]);
			widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
			break;
		case COLOR_YELLOW:
    			sprintf (widget_path, "/%s/Yellow", hildonToolbars[toolbar]);
			widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
			break;
		case COLOR_ORANGE:
    			sprintf (widget_path, "/%s/Orange", hildonToolbars[toolbar]);
			widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
			break;
		case COLOR_MAGENTA:
    			sprintf (widget_path, "/%s/Fuchsia", hildonToolbars[toolbar]);
			widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
			break;
		default:
			widget = NULL;
	}

	if (widget) {
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(widget), TRUE);
	}
}

void hildon_toolbar_switch_do (gint toolbar_no)
{
  if (switch_to_toolbar_landscape != NULL) {
    return;
  }

  switch_to_toolbar_portrait = hildon_cur_toolbar_portrait;
  switch_to_toolbar_landscape = hildon_cur_toolbar_landscape;

  if (hildon_cur_toolbar_visible) {
    if (device_is_portrait_mode ()) {
      gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_portrait));
      gtk_widget_show (GTK_WIDGET(hildon_toolbars[toolbar_no+1]));
    } else {
      gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_landscape));
      gtk_widget_show (GTK_WIDGET(hildon_toolbars[toolbar_no]));
    }
  }

  hildon_cur_toolbar_landscape = hildon_toolbars[toolbar_no];
  hildon_cur_toolbar_portrait = hildon_toolbars[toolbar_no+1];
}

void hildon_toolbar_switch_back ()
{
  if (switch_to_toolbar_landscape == NULL) {
    return;
  }

  if (hildon_cur_toolbar_visible) {
    if (device_is_portrait_mode ()) {
      gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_portrait));
      gtk_widget_show (GTK_WIDGET(switch_to_toolbar_portrait));
    } else {
      gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_landscape));
      gtk_widget_show (GTK_WIDGET(switch_to_toolbar_landscape));
    }
  }

  hildon_cur_toolbar_landscape = switch_to_toolbar_landscape;
  hildon_cur_toolbar_portrait = switch_to_toolbar_portrait;

  switch_to_toolbar_landscape = NULL;
  switch_to_toolbar_portrait = NULL;
}

void hildon_toolbar_switch (GtkWidget *item, gpointer user_data)
{
  gint toolbar_no = (gint) user_data;

  if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON(item))) {
    hildon_toolbar_switch_do (toolbar_no);
  } else {
    hildon_toolbar_switch_back ();
  }
}

void hildon_draw_switch (GtkWidget *item, gpointer user_data)
{
  hildon_toolbar_switch_back ();

  if (device_is_portrait_mode ()) {
     gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_portrait));
     gtk_widget_show (GTK_WIDGET(hildon_toolbars[HILDON_TOOLBAR_DRAW_PORTRAIT]));
  } else {
     gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_landscape));
     gtk_widget_show (GTK_WIDGET(hildon_toolbars[HILDON_TOOLBAR_DRAW_LANDSCAPE]));
  }

  hildon_cur_toolbar_portrait = hildon_toolbars[HILDON_TOOLBAR_DRAW_PORTRAIT];
  hildon_cur_toolbar_landscape = hildon_toolbars[HILDON_TOOLBAR_DRAW_LANDSCAPE];

  hildon_cur_toolbar_visible = TRUE;
}

void hildon_pdf_switch (GtkWidget *item, gpointer user_data)
{
  hildon_toolbar_switch_back ();

  if (device_is_portrait_mode ()) {
     gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_portrait));
     gtk_widget_show (GTK_WIDGET(hildon_toolbars[HILDON_TOOLBAR_PDF_PORTRAIT]));
  } else {
     gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_landscape));
     gtk_widget_show (GTK_WIDGET(hildon_toolbars[HILDON_TOOLBAR_PDF_LANDSCAPE]));
  }

  hildon_cur_toolbar_portrait = hildon_toolbars[HILDON_TOOLBAR_PDF_PORTRAIT];
  hildon_cur_toolbar_landscape = hildon_toolbars[HILDON_TOOLBAR_PDF_LANDSCAPE];

  hildon_cur_toolbar_visible = TRUE;
}

void hildon_note_switch (GtkWidget *item, gpointer user_data)
{
  hildon_toolbar_switch_back ();

  if (device_is_portrait_mode ()) {
     gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_portrait));
     gtk_widget_show (GTK_WIDGET(hildon_toolbars[HILDON_TOOLBAR_NOTE_PORTRAIT]));
  } else {
     gtk_widget_hide (GTK_WIDGET(hildon_cur_toolbar_landscape));
     gtk_widget_show (GTK_WIDGET(hildon_toolbars[HILDON_TOOLBAR_NOTE_LANDSCAPE]));
  }

  hildon_cur_toolbar_portrait = hildon_toolbars[HILDON_TOOLBAR_NOTE_PORTRAIT];
  hildon_cur_toolbar_landscape = hildon_toolbars[HILDON_TOOLBAR_NOTE_LANDSCAPE];

  hildon_cur_toolbar_visible = TRUE;
}

void hildon_file_menu_activate (GtkWidget *item, gpointer user_data)
{
	int i,num_items;

	num_items = sizeof (fremantle_menu_tools_entries)/
			sizeof (XournalHildonMenu);
	for (i=0;i<num_items;i++) {
		gtk_widget_hide (GTK_WIDGET(fremantle_menu_tools_entries[i].widget));
	}

	num_items = sizeof (fremantle_menu_file_entries)/
			sizeof (XournalHildonMenu);
	for (i=0;i<num_items;i++) {
		gtk_widget_show (GTK_WIDGET(fremantle_menu_file_entries[i].widget));
	}
}

void hildon_tools_menu_activate (GtkWidget *item, gpointer user_data)
{
	int i,num_items;

	num_items = sizeof (fremantle_menu_file_entries)/
			sizeof (XournalHildonMenu);
	for (i=0;i<num_items;i++) {
		gtk_widget_hide (GTK_WIDGET(fremantle_menu_file_entries[i].widget));
	}

	num_items = sizeof (fremantle_menu_tools_entries)/
			sizeof (XournalHildonMenu);
	for (i=0;i<num_items;i++) {
		gtk_widget_show (GTK_WIDGET(fremantle_menu_tools_entries[i].widget));
	}
}

GtkWidget*
create_winMain (osso_context_t *ctx)
{
  HildonProgram *hildon_program;
  HildonWindow *hildon_window;
#ifdef MAEMO_4
  GtkWidget *hildon_mainmenu;
#else
  HildonAppMenu *hildon_mainmenu;
#endif
  GtkWidget *vboxMain;
  GtkWidget *scrolledwindowMain;
  GtkWidget *hildonSelector;
  GtkAccelGroup *accel_group;
  GtkActionGroup *action_group;
  GtkAction *action_eraser, *action_pen;
  GtkWidget *widget_eraser, *widget_pen;
  GtkWidget *undo_widget;
  GtkWidget *redo_widget;
  GtkWidget *widget;
  GtkToolItem *colorItem;
  GError *error;
  gchar *label;
  char xml_path[2048];
  int i;

  register_hildon_stock_icons ();

  hildon_program = HILDON_PROGRAM (hildon_program_get_instance());
  g_set_application_name (_("Xournal"));

  /* todo: diablo
  hildon_window = HILDON_WINDOW (hildon_window_new ());
  */
  hildon_window = HILDON_WINDOW (hildon_stackable_window_new ());

  hildon_program_add_window (hildon_program, hildon_window);

  action_group = gtk_action_group_new ("HildonActions");

  gtk_action_group_add_actions (action_group, hildon_menu_entries,
		  G_N_ELEMENTS (hildon_menu_entries), hildon_window);

  gtk_action_group_add_actions (action_group, hildon_toggle_entries,
		  G_N_ELEMENTS (hildon_toggle_entries), hildon_window);

  gtk_action_group_add_radio_actions (action_group, hildon_view_radio_entries,
		  G_N_ELEMENTS(hildon_view_radio_entries), 0, NULL, hildon_window);

  gtk_action_group_add_radio_actions (action_group, hildon_paperColor_radio_entries,
		  G_N_ELEMENTS(hildon_paperColor_radio_entries), 0, NULL, hildon_window);

  gtk_action_group_add_radio_actions (action_group, hildon_paperStyle_radio_entries,
		  G_N_ELEMENTS(hildon_paperStyle_radio_entries), 1, NULL, hildon_window);

  gtk_action_group_add_radio_actions (action_group, hildon_tools_radio_entries,
		  G_N_ELEMENTS(hildon_tools_radio_entries), 0, G_CALLBACK(hildon_tools_radio_action), hildon_window);

  gtk_action_group_add_toggle_actions (action_group, hildon_toolsOther_toggle_entries,
		  G_N_ELEMENTS(hildon_toolsOther_toggle_entries), hildon_window);

  gtk_action_group_add_radio_actions (action_group, hildon_penOptions_radio_entries,
		  G_N_ELEMENTS(hildon_penOptions_radio_entries), 2, G_CALLBACK(hildon_penOptions_radio_action), hildon_window);

  gtk_action_group_add_radio_actions (action_group, hildon_penColor_radio_entries,
		  G_N_ELEMENTS(hildon_penColor_radio_entries), 3, G_CALLBACK(hildon_penColor_radio_action), hildon_window);

  gtk_action_group_add_radio_actions (action_group,
		  hildon_eraserThicknessOptions_radio_entries,
		  G_N_ELEMENTS(hildon_eraserThicknessOptions_radio_entries), 1, NULL,
		  hildon_window);

  gtk_action_group_add_radio_actions (action_group,
		  hildon_eraserTypeOptions_radio_entries,
		  G_N_ELEMENTS(hildon_eraserTypeOptions_radio_entries), 0, NULL,
		  hildon_window);

  gtk_action_group_add_radio_actions (action_group,
		  hildon_highlighterThicknessOptions_radio_entries,
		  G_N_ELEMENTS(hildon_highlighterThicknessOptions_radio_entries), 1, NULL,
		  hildon_window);

  ui_manager = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

  accel_group = gtk_ui_manager_get_accel_group (ui_manager);
  gtk_window_add_accel_group (GTK_WINDOW (hildon_window), accel_group);

  error = NULL;
  sprintf (xml_path, PACKAGE_UIXML_DIR"/xo-hildon_ui.xml");
  
  if (!gtk_ui_manager_add_ui_from_file (ui_manager, xml_path, &error)) {
	printf ("buildind menus failed: %s", error->message);
	g_error_free (error);
	exit (-1);
  }

  vboxMain = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (hildon_window), vboxMain);

//#ifdef USE_HILDON
//  scrolledwindowMain = hildon_pannable_area_new ();
//  g_object_set (scrolledwindowMain, "mov-mode", !HILDON_MOVEMENT_MODE_VERT | !HILDON_MOVEMENT_MODE_HORIZ);
//#else
  scrolledwindowMain = gtk_scrolled_window_new (NULL, NULL);
//#endif
  gtk_widget_hide (gtk_scrolled_window_get_hscrollbar (GTK_SCROLLED_WINDOW(scrolledwindowMain)));
  gtk_widget_hide (gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW(scrolledwindowMain)));

  gtk_box_pack_start (GTK_BOX (vboxMain), scrolledwindowMain, TRUE, TRUE, 0);

  GLADE_HOOKUP_OBJECT (GTK_WIDGET(hildon_window),
	 scrolledwindowMain, "scrolledwindowMain");

//  gtk_widget_show_all (GTK_WIDGET (hildon_mainmenu));
  gtk_widget_show_all (GTK_WIDGET (hildon_window));

  /* Load toolbars */
  hildon_toolbars[HILDON_TOOLBAR_NOTE_PORTRAIT] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarNotePortrait");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_NOTE_PORTRAIT]));
  hildon_toolbars[HILDON_TOOLBAR_NOTE_LANDSCAPE] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarNoteLandscape");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_NOTE_LANDSCAPE]));

  hildon_toolbars[HILDON_TOOLBAR_PDF_PORTRAIT] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPdfPortrait");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_PDF_PORTRAIT]));
  hildon_toolbars[HILDON_TOOLBAR_PDF_LANDSCAPE] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPdfLandscape");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_PDF_LANDSCAPE]));

  hildon_toolbars[HILDON_TOOLBAR_DRAW_PORTRAIT] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarDrawPortrait");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_DRAW_PORTRAIT]));
  hildon_toolbars[HILDON_TOOLBAR_DRAW_LANDSCAPE] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarDrawLandscape");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_DRAW_LANDSCAPE]));

  hildon_toolbars[HILDON_TOOLBAR_HAND_PORTRAIT] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarHandPortrait");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_HAND_PORTRAIT]));
  hildon_toolbars[HILDON_TOOLBAR_HAND_LANDSCAPE] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarHandLandscape");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_HAND_LANDSCAPE]));

  hildon_toolbars[HILDON_TOOLBAR_PEN_PORTRAIT] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPenPortrait");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_PEN_PORTRAIT]));
  hildon_toolbars[HILDON_TOOLBAR_PEN_LANDSCAPE] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPenLandscape");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_PEN_LANDSCAPE]));

  hildon_toolbars[HILDON_TOOLBAR_COLOR_PORTRAIT] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPenColorPortrait");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_COLOR_PORTRAIT]));
  hildon_toolbars[HILDON_TOOLBAR_COLOR_LANDSCAPE] = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPenColorLandscape");
  hildon_window_add_toolbar (hildon_window, GTK_TOOLBAR (hildon_toolbars[HILDON_TOOLBAR_COLOR_LANDSCAPE]));

  for (i=0;i<HILDON_NUM_TOOLBARS;i++) {
    char widget_path[256];

    sprintf (widget_path, "/%s/Hand", hildonToolbars[i]);

    widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
    g_signal_connect (G_OBJECT (widget), "clicked",
	G_CALLBACK (hildon_toolbar_switch), (gpointer)HILDON_TOOLBAR_HAND_LANDSCAPE);

    sprintf (widget_path, "/%s/Pen", hildonToolbars[i]);

    widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
    g_signal_connect (G_OBJECT (widget), "clicked",
	G_CALLBACK (hildon_toolbar_switch), (gpointer)HILDON_TOOLBAR_PEN_LANDSCAPE);

    sprintf (widget_path, "/%s/PenColor", hildonToolbars[i]);

    widget = gtk_ui_manager_get_widget (ui_manager, widget_path);
    g_signal_connect (G_OBJECT (widget), "clicked",
	G_CALLBACK (hildon_toolbar_switch), (gpointer)HILDON_TOOLBAR_COLOR_LANDSCAPE);
  }

  g_signal_connect (G_OBJECT(hildon_toolbars[HILDON_TOOLBAR_COLOR_PORTRAIT]), "show",
	G_CALLBACK (hildon_toolbar_refresh_color), (gpointer)HILDON_TOOLBAR_COLOR_PORTRAIT);
  g_signal_connect (G_OBJECT(hildon_toolbars[HILDON_TOOLBAR_COLOR_LANDSCAPE]), "show",
	G_CALLBACK (hildon_toolbar_refresh_color), (gpointer)HILDON_TOOLBAR_COLOR_LANDSCAPE);

  // Undo / Redo buttons
  undo_widget = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarNoteLandscape/Undo");
  redo_widget = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarNoteLandscape/Redo");
  hildon_undo[HILDON_TOOLBAR_NOTE_LANDSCAPE] = undo_widget;
  hildon_redo[HILDON_TOOLBAR_NOTE_LANDSCAPE] = redo_widget;

  undo_widget = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPdfLandscape/Undo");
  redo_widget = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarPdfLandscape/Redo");
  hildon_undo[HILDON_TOOLBAR_PDF_LANDSCAPE] = undo_widget;
  hildon_redo[HILDON_TOOLBAR_PDF_LANDSCAPE] = redo_widget;

  undo_widget = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarDrawLandscape/Undo");
  redo_widget = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBarDrawLandscape/Redo");
  hildon_undo[HILDON_TOOLBAR_DRAW_LANDSCAPE] = undo_widget;
  hildon_redo[HILDON_TOOLBAR_DRAW_LANDSCAPE] = redo_widget;

  pickerButton = hildon_picker_button_new (HILDON_SIZE_AUTO,
		  HILDON_BUTTON_ARRANGEMENT_VERTICAL);
  hildonSelector = hildon_touch_selector_new_text ();

  hildon_button_set_title (HILDON_BUTTON(pickerButton), _("Select layer"));

  hildon_touch_selector_set_column_selection_mode (
		HILDON_TOUCH_SELECTOR(hildonSelector),
 		HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE);

  g_signal_connect (G_OBJECT (hildonSelector), "changed",
		G_CALLBACK (on_comboLayer_changed), (gpointer) "maemo");

  /* Set a default [not yet configurable] toolbar */
  hildon_cur_toolbar_landscape = hildon_toolbars[HILDON_TOOLBAR_NOTE_LANDSCAPE];
  hildon_cur_toolbar_portrait = hildon_toolbars[HILDON_TOOLBAR_NOTE_PORTRAIT];

  /* Hide all */
  for (i=0;i<HILDON_NUM_TOOLBARS;i++) {
	gtk_widget_hide_all (GTK_WIDGET (hildon_toolbars[i]));
  }

  hildon_picker_button_set_selector (HILDON_PICKER_BUTTON(pickerButton),
		HILDON_TOUCH_SELECTOR(hildonSelector));

  hildon_app_menu_append (hildon_mainmenu, GTK_BUTTON(pickerButton));
  gtk_widget_show_all (pickerButton);

  fremantle_menu_file_entries[HILDON_SHOW_LAYER_POS].widget = GTK_WIDGET (pickerButton);

  g_signal_connect ((gpointer) hildon_window, "delete_event",
		G_CALLBACK (on_winMain_delete_event),
		NULL);

#ifdef MAEMO_4
  hildon_mainmenu = gtk_ui_manager_get_widget (ui_manager, "/HildonMainMenu");
  hildon_window_set_menu (hildon_window, GTK_MENU (hildon_mainmenu));
#else
  hildon_mainmenu = hildon_create_menu(hildon_window);
  hildon_window_set_app_menu (hildon_window, hildon_mainmenu);
#endif

  return GTK_WIDGET (hildon_window);
}

GtkWidget*
create_papersizeDialog (void)
{
  GtkWidget *papersizeDialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox2;
  GtkWidget *labelStdSizes;
  GtkWidget *comboStdSizes;
  GtkWidget *hbox3;
  GtkWidget *labelWidth;
  GtkWidget *entryWidth;
  GtkWidget *labelHeight;
  GtkWidget *entryHeight;
  GtkWidget *comboUnit;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  papersizeDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (papersizeDialog), "Set Paper Size");
  gtk_window_set_modal (GTK_WINDOW (papersizeDialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (papersizeDialog), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (papersizeDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (papersizeDialog)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox2, TRUE, TRUE, 10);

  labelStdSizes = gtk_label_new ("Standard paper sizes:");
  gtk_widget_show (labelStdSizes);
  gtk_box_pack_start (GTK_BOX (hbox2), labelStdSizes, FALSE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (labelStdSizes), 10, 0);

  comboStdSizes = gtk_combo_box_new_text ();
  gtk_widget_show (comboStdSizes);
  gtk_box_pack_start (GTK_BOX (hbox2), comboStdSizes, TRUE, TRUE, 5);
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboStdSizes), "A4");
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboStdSizes), "A4 (landscape)");
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboStdSizes), "US Letter");
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboStdSizes), "US Letter (landscape)");
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboStdSizes), "Custom");

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox3, TRUE, TRUE, 8);

  labelWidth = gtk_label_new ("Width:");
  gtk_widget_show (labelWidth);
  gtk_box_pack_start (GTK_BOX (hbox3), labelWidth, FALSE, FALSE, 10);

  entryWidth = gtk_entry_new ();
  gtk_widget_show (entryWidth);
  gtk_box_pack_start (GTK_BOX (hbox3), entryWidth, TRUE, TRUE, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entryWidth), 5);

  labelHeight = gtk_label_new ("Height:");
  gtk_widget_show (labelHeight);
  gtk_box_pack_start (GTK_BOX (hbox3), labelHeight, FALSE, FALSE, 10);

  entryHeight = gtk_entry_new ();
  gtk_widget_show (entryHeight);
  gtk_box_pack_start (GTK_BOX (hbox3), entryHeight, TRUE, TRUE, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entryHeight), 5);

  comboUnit = gtk_combo_box_new_text ();
  gtk_widget_show (comboUnit);
  gtk_box_pack_start (GTK_BOX (hbox3), comboUnit, FALSE, TRUE, 8);
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboUnit), "cm");
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboUnit), "in");
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboUnit), "pixels");
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboUnit), "points");

  dialog_action_area1 = GTK_DIALOG (papersizeDialog)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (papersizeDialog), cancelbutton1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (papersizeDialog), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) comboStdSizes, "changed",
                    G_CALLBACK (on_comboStdSizes_changed),
                    NULL);
  g_signal_connect ((gpointer) entryWidth, "changed",
                    G_CALLBACK (on_entryWidth_changed),
                    NULL);
  g_signal_connect ((gpointer) entryHeight, "changed",
                    G_CALLBACK (on_entryHeight_changed),
                    NULL);
  g_signal_connect ((gpointer) comboUnit, "changed",
                    G_CALLBACK (on_comboUnit_changed),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (papersizeDialog, papersizeDialog, "papersizeDialog");
  GLADE_HOOKUP_OBJECT_NO_REF (papersizeDialog, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (papersizeDialog, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (papersizeDialog, labelStdSizes, "labelStdSizes");
  GLADE_HOOKUP_OBJECT (papersizeDialog, comboStdSizes, "comboStdSizes");
  GLADE_HOOKUP_OBJECT (papersizeDialog, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (papersizeDialog, labelWidth, "labelWidth");
  GLADE_HOOKUP_OBJECT (papersizeDialog, entryWidth, "entryWidth");
  GLADE_HOOKUP_OBJECT (papersizeDialog, labelHeight, "labelHeight");
  GLADE_HOOKUP_OBJECT (papersizeDialog, entryHeight, "entryHeight");
  GLADE_HOOKUP_OBJECT (papersizeDialog, comboUnit, "comboUnit");
  GLADE_HOOKUP_OBJECT_NO_REF (papersizeDialog, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (papersizeDialog, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (papersizeDialog, okbutton1, "okbutton1");

  return papersizeDialog;
}

GtkWidget*
create_aboutDialog (void)
{
  GtkWidget *aboutDialog;
  GtkWidget *dialog_vbox2;
  GtkWidget *image387;
  GtkWidget *labelTitle;
  GtkWidget *labelInfo;
  GtkWidget *dialog_action_area2;
  GtkWidget *closebutton1;

  aboutDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (aboutDialog), _("About Xournal"));
  gtk_window_set_resizable (GTK_WINDOW (aboutDialog), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (aboutDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox2 = GTK_DIALOG (aboutDialog)->vbox;
  gtk_widget_show (dialog_vbox2);

  image387 = create_pixmap (aboutDialog, "xournal.png");
  gtk_widget_show (image387);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), image387, FALSE, TRUE, 12);

  labelTitle = gtk_label_new (_("Xournal"));
  gtk_widget_show (labelTitle);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), labelTitle, FALSE, FALSE, 3);
  gtk_label_set_justify (GTK_LABEL (labelTitle), GTK_JUSTIFY_CENTER);

  labelInfo = gtk_label_new (_("Written by Denis Auroux       http://xournal.sourceforge.net/       "));
  gtk_widget_show (labelInfo);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), labelInfo, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (labelInfo), GTK_JUSTIFY_CENTER);
  gtk_misc_set_padding (GTK_MISC (labelInfo), 20, 10);

  dialog_action_area2 = GTK_DIALOG (aboutDialog)->action_area;
  gtk_widget_show (dialog_action_area2);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);

  closebutton1 = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (closebutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (aboutDialog), closebutton1, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (closebutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (aboutDialog, aboutDialog, "aboutDialog");
  GLADE_HOOKUP_OBJECT_NO_REF (aboutDialog, dialog_vbox2, "dialog_vbox2");
  GLADE_HOOKUP_OBJECT (aboutDialog, image387, "image387");
  GLADE_HOOKUP_OBJECT (aboutDialog, labelTitle, "labelTitle");
  GLADE_HOOKUP_OBJECT (aboutDialog, labelInfo, "labelInfo");
  GLADE_HOOKUP_OBJECT_NO_REF (aboutDialog, dialog_action_area2, "dialog_action_area2");
  GLADE_HOOKUP_OBJECT (aboutDialog, closebutton1, "closebutton1");

  return aboutDialog;
}

GtkWidget*
create_zoomDialog (void)
{
  GtkWidget *zoomDialog;
  GtkWidget *dialog_vbox3;
  GtkWidget *vbox1;
  GtkWidget *hbox4;
  GtkWidget *radioZoom;
  GSList *radioZoom_group = NULL;
  GtkObject *spinZoom_adj;
  GtkWidget *spinZoom;
  GtkWidget *label1;
  GtkWidget *radioZoom100;
  GtkWidget *radioZoomWidth;
  GtkWidget *radioZoomHeight;
  GtkWidget *dialog_action_area3;
  GtkWidget *cancelbutton2;
  GtkWidget *button1;
  GtkWidget *button2;

  zoomDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (zoomDialog), _("Set Zoom"));
  gtk_window_set_modal (GTK_WINDOW (zoomDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (zoomDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox3 = GTK_DIALOG (zoomDialog)->vbox;
  gtk_widget_show (dialog_vbox3);

  vbox1 = gtk_vbox_new (FALSE, 2);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox3), vbox1, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 8);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox4, FALSE, FALSE, 0);

  radioZoom = gtk_radio_button_new_with_mnemonic (NULL, _("Zoom: "));
  gtk_widget_show (radioZoom);
  gtk_box_pack_start (GTK_BOX (hbox4), radioZoom, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (radioZoom), 4);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioZoom), radioZoom_group);
  radioZoom_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioZoom));

  spinZoom_adj = gtk_adjustment_new (100, 10, 1500, 5, 20, 20);
  spinZoom = gtk_spin_button_new (GTK_ADJUSTMENT (spinZoom_adj), 1, 0);
  gtk_widget_show (spinZoom);
  gtk_box_pack_start (GTK_BOX (hbox4), spinZoom, FALSE, TRUE, 5);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinZoom), TRUE);

  label1 = gtk_label_new ("%");
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox4), label1, FALSE, TRUE, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0.48, 0.5);

  radioZoom100 = gtk_radio_button_new_with_mnemonic (NULL, _("Normal size (100%)"));
  gtk_widget_show (radioZoom100);
  gtk_box_pack_start (GTK_BOX (vbox1), radioZoom100, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (radioZoom100), 4);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioZoom100), radioZoom_group);
  radioZoom_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioZoom100));

  radioZoomWidth = gtk_radio_button_new_with_mnemonic (NULL, _("Page Width"));
  gtk_widget_show (radioZoomWidth);
  gtk_box_pack_start (GTK_BOX (vbox1), radioZoomWidth, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (radioZoomWidth), 4);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioZoomWidth), radioZoom_group);
  radioZoom_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioZoomWidth));

  radioZoomHeight = gtk_radio_button_new_with_mnemonic (NULL, _("Page Height"));
  gtk_widget_show (radioZoomHeight);
  gtk_box_pack_start (GTK_BOX (vbox1), radioZoomHeight, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (radioZoomHeight), 4);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radioZoomHeight), radioZoom_group);
  radioZoom_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radioZoomHeight));

  dialog_action_area3 = GTK_DIALOG (zoomDialog)->action_area;
  gtk_widget_show (dialog_action_area3);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area3), GTK_BUTTONBOX_END);

  cancelbutton2 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton2);
  gtk_dialog_add_action_widget (GTK_DIALOG (zoomDialog), cancelbutton2, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton2, GTK_CAN_DEFAULT);

  button1 = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (button1);
  gtk_dialog_add_action_widget (GTK_DIALOG (zoomDialog), button1, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  button2 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (button2);
  gtk_dialog_add_action_widget (GTK_DIALOG (zoomDialog), button2, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) radioZoom, "toggled",
                    G_CALLBACK (on_radioZoom_toggled),
                    NULL);
  g_signal_connect ((gpointer) spinZoom, "value_changed",
                    G_CALLBACK (on_spinZoom_value_changed),
                    NULL);
  g_signal_connect ((gpointer) radioZoom100, "toggled",
                    G_CALLBACK (on_radioZoom100_toggled),
                    NULL);
  g_signal_connect ((gpointer) radioZoomWidth, "toggled",
                    G_CALLBACK (on_radioZoomWidth_toggled),
                    NULL);
  g_signal_connect ((gpointer) radioZoomHeight, "toggled",
                    G_CALLBACK (on_radioZoomHeight_toggled),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (zoomDialog, zoomDialog, "zoomDialog");
  GLADE_HOOKUP_OBJECT_NO_REF (zoomDialog, dialog_vbox3, "dialog_vbox3");
  GLADE_HOOKUP_OBJECT (zoomDialog, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (zoomDialog, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (zoomDialog, radioZoom, "radioZoom");
  GLADE_HOOKUP_OBJECT (zoomDialog, spinZoom, "spinZoom");
  GLADE_HOOKUP_OBJECT (zoomDialog, label1, "label1");
  GLADE_HOOKUP_OBJECT (zoomDialog, radioZoom100, "radioZoom100");
  GLADE_HOOKUP_OBJECT (zoomDialog, radioZoomWidth, "radioZoomWidth");
  GLADE_HOOKUP_OBJECT (zoomDialog, radioZoomHeight, "radioZoomHeight");
  GLADE_HOOKUP_OBJECT_NO_REF (zoomDialog, dialog_action_area3, "dialog_action_area3");
  GLADE_HOOKUP_OBJECT (zoomDialog, cancelbutton2, "cancelbutton2");
  GLADE_HOOKUP_OBJECT (zoomDialog, button1, "button1");
  GLADE_HOOKUP_OBJECT (zoomDialog, button2, "button2");

  return zoomDialog;
}

HildonAppMenu *
hildon_create_stack_menu (void)
{
	HildonAppMenu *menu = HILDON_APP_MENU(hildon_app_menu_new ());
	GtkWidget *button;
	
	button = hildon_button_new_with_text (HILDON_SIZE_AUTO,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
		_("Tools"), NULL);
	gtk_widget_show (button);
	g_signal_connect_after (button, "clicked",
		G_CALLBACK (hildon_show_view),
		tools_view);
	hildon_app_menu_append (menu, GTK_BUTTON(button));
	
	button = hildon_button_new_with_text (HILDON_SIZE_AUTO,
		HILDON_BUTTON_ARRANGEMENT_VERTICAL,
		_("Journal"), NULL);
	gtk_widget_show (button);
	g_signal_connect_after (button, "clicked",
		G_CALLBACK (hildon_show_view),
		journal_view);
	hildon_app_menu_append (menu, GTK_BUTTON(button));

	button = hildon_button_new_with_text (HILDON_SIZE_AUTO,
		HILDON_BUTTON_ARRANGEMENT_VERTICAL,
		_("Settings"), NULL);
	gtk_widget_show (button);
	g_signal_connect_after (button, "clicked",
		G_CALLBACK (hildon_show_view),
		settings_view);
	hildon_app_menu_append (menu, GTK_BUTTON(button));

	return menu;
}

GtkWidget *
hildon_create_tools_view (void)
{
	GtkWidget *win;
	GtkWidget *vboxMain;
	GtkWidget *label;
	GtkWidget *buttonFine;
	GtkWidget *buttonMedium;
	GtkWidget *buttonThick;
	GtkWidget *toolbarPen;
	GtkWidget *tmp_image;
	GSList *buttonFine_group = NULL;

	win = hildon_stackable_window_new ();
	gtk_window_set_title (GTK_WINDOW(win), _("Tools"));

	vboxMain = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER(win), GTK_WIDGET(vboxMain));

	toolbarPen = gtk_toolbar_new ();
	gtk_widget_show (toolbarPen);
	gtk_box_pack_start (GTK_BOX (vboxMain), toolbarPen, FALSE, FALSE, 0);

	buttonFine = (GtkWidget*) gtk_radio_tool_button_new (NULL);
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (buttonFine), _("Fine"));
	tmp_image = create_pixmap (GTK_WIDGET(win), "thin.png");
	gtk_widget_show (tmp_image);
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (buttonFine), tmp_image);
	gtk_widget_show (buttonFine);
	gtk_tool_item_set_homogeneous (GTK_TOOL_ITEM (buttonFine), FALSE);
	gtk_container_add (GTK_CONTAINER (toolbarPen), buttonFine);
//	gtk_widget_set_size_request (buttonFine, 24, -1);
	gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON (buttonFine), buttonFine_group);
	buttonFine_group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (buttonFine));

	buttonMedium = (GtkWidget*) gtk_radio_tool_button_new (NULL);
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (buttonMedium), _("Medium"));
	tmp_image = create_pixmap (GTK_WIDGET(win), "medium.png");
	gtk_widget_show (tmp_image);
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (buttonMedium), tmp_image);
	gtk_widget_show (buttonMedium);
	gtk_tool_item_set_homogeneous (GTK_TOOL_ITEM (buttonMedium), FALSE);
	gtk_container_add (GTK_CONTAINER (toolbarPen), buttonMedium);
//	gtk_widget_set_size_request (buttonMedium, 24, -1);
	gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON (buttonMedium), buttonFine_group);
	buttonFine_group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (buttonMedium));

	buttonThick = (GtkWidget*) gtk_radio_tool_button_new (NULL);
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (buttonThick), _("Thick"));
	tmp_image = create_pixmap (GTK_WIDGET(win), "thick.png");
	gtk_widget_show (tmp_image);
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (buttonThick), tmp_image);
	gtk_widget_show (buttonThick);
	gtk_tool_item_set_homogeneous (GTK_TOOL_ITEM (buttonThick), FALSE);
	gtk_container_add (GTK_CONTAINER (toolbarPen), buttonThick);
//	gtk_widget_set_size_request (buttonThick, 24, -1);
	gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON (buttonThick), buttonFine_group);
	buttonFine_group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (buttonThick));

	return win;
}

GtkWidget *
hildon_create_journal_view (void)
{
	GtkWidget *win;
	GtkWidget *box;
	GtkWidget *label;

	win = hildon_stackable_window_new ();
	gtk_window_set_title (GTK_WINDOW(win), _("Journal view"));

	label = gtk_label_new ("journal view");
	gtk_widget_show (label);

	box = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (box);

	gtk_box_pack_start (GTK_BOX(box), label, TRUE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER(win), GTK_WIDGET(box));

	return win;
}

GtkWidget *
hildon_create_settings_view (void)
{
	GtkWidget *win;
	GtkWidget *box;
	GtkWidget *label;

	win = hildon_stackable_window_new ();
	gtk_window_set_title (GTK_WINDOW(win), _("Settings view"));

	label = gtk_label_new ("settings view");
	gtk_widget_show (label);

	box = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (box);

	gtk_box_pack_start (GTK_BOX(box), label, TRUE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER(win), GTK_WIDGET(box));

	return win;
}

void
hildon_grab_volume_keys (int val)
{
	Atom atom;
	GdkDisplay *display = NULL;

	display = gdk_drawable_get_display (winMain->window);
	atom = gdk_x11_get_xatom_by_name_for_display (display, "_HILDON_ZOOM_KEY_ATOM");
	XChangeProperty (GDK_DISPLAY_XDISPLAY (display),
			GDK_WINDOW_XID(winMain->window), atom, XA_INTEGER, 32,
			PropModeReplace, (unsigned char *) &val, 1);
}

void
hildon_rotate_ui (const gchar *mode)
{
	GtkWidget *hildon_toolbar;

	if (!strcmp (mode, "portrait")) {
		if (hildon_cur_toolbar_visible) {
			// hide landscape toolbar
			gtk_widget_hide (hildon_cur_toolbar_landscape);
			// show portrait toolbar
			gtk_widget_show (hildon_cur_toolbar_portrait);
		}
	}

	if (!strcmp (mode, "landscape")) {
		if (hildon_cur_toolbar_visible) {
			// hide portrait toolbar
			gtk_widget_hide (hildon_cur_toolbar_portrait);
			// show landscape toolbar
			gtk_widget_show (hildon_cur_toolbar_landscape);
		}
	}
}

void
hildon_orientationChanged_process (GdkScreen *screen, GtkWidget *widget)
{
	if (device_is_portrait_mode ()) {
		hildon_rotate_ui ("portrait");
	} else {
		hildon_rotate_ui ("landscape");
	}
}

gboolean
device_is_portrait_mode()
{
	GdkScreen *screen = gtk_widget_get_screen (winMain);
	int width = gdk_screen_get_width (screen);
	int height = gdk_screen_get_height (screen);

	if (width > height) {
		return FALSE;
	} else {
		return TRUE;
	}
}

/*
gint
hildon_button_pressed_timedout (gpointer data) {
	GdkEventButton *event = (GdkEventButton *)data;

	if (two_half_button_click) {
		return;
	}

	hildon_button_pressed = 0;
	hildon_button_released = 0;
	hildon_timer_running_already = 0;

	printf ("Timer expired\n");

	return 0;
}
*/
