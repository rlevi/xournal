#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/stat.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libgnomecanvas/libgnomecanvas.h>
#ifdef USE_HILDON
#include <libosso.h>
#include <libintl.h>
#include <locale.h>
#include <zlib.h>
#include <errno.h>
#include <hildon/hildon-note.h>
#include <dbus/dbus.h>
#include <mce/mode-names.h>
#include <mce/dbus-names.h>
#include "xo-hildon_ui.h"
#define MCE_SIGNAL_MATCH "type='signal',"  \
	"sender='"    MCE_SERVICE                "'," \
	"path='"      MCE_SIGNAL_PATH            "'," \
	"interface='" MCE_SIGNAL_IF              "'"
#endif

#include "xournal.h"
#ifndef USE_HILDON
#include "xo-interface.h"
#endif
#include "xo-support.h"
#include "xo-callbacks.h"
#include "xo-misc.h"
#include "xo-file.h"
#include "xo-paint.h"
#include "xo-shapes.h"

GtkWidget *winMain;
GnomeCanvas *canvas;
GtkUIManager *ui_manager;

struct Journal journal; // the journal
struct BgPdf bgpdf;  // the PDF loader stuff
struct UIData ui;   // the user interface data
struct UndoItem *undo, *redo; // the undo and redo stacks

double DEFAULT_ZOOM;

#ifdef USE_HILDON
osso_context_t *ctx;
char *theme_name;
int grab_volume_keys;
gboolean autosave_disabled;
GtkWidget *hildon_toolbars[HILDON_NUM_TOOLBARS];
#endif

void init_stuff (int argc, char *argv[])
{
  GtkWidget *w;
  GList *dev_list;
  GdkDevice *device;
  GdkScreen *screen;
  int i, j;
  struct Brush *b;
  gboolean can_xinput, success;
  gchar *tmppath, *tmpfn;
#ifdef USE_HILDON
  gzFile f;
  gchar *autosave_filename;
#endif

#ifndef USE_HILDON
  // create some data structures needed to populate the preferences
  ui.default_page.bg = g_new(struct Background, 1);

  // initialize config file names
  tmppath = g_build_filename(g_get_home_dir(), CONFIG_DIR, NULL);
  mkdir(tmppath, 0700); // safer (MRU data may be confidential)
  ui.mrufile = g_build_filename(tmppath, MRU_FILE, NULL);
  ui.configfile = g_build_filename(tmppath, CONFIG_FILE, NULL);
  g_free(tmppath);

  // initialize preferences
  init_config_default();
  load_config_from_file();
#endif
  ui.font_name = g_strdup(ui.default_font_name);
  ui.font_size = ui.default_font_size;
  ui.hiliter_alpha_mask = 0xffffff00 + (guint)(255*ui.hiliter_opacity);

  // we need an empty canvas prior to creating the journal structures
  canvas = GNOME_CANVAS (gnome_canvas_new_aa ());

  // initialize data
  ui.default_page.bg->canvas_item = NULL;
  ui.layerbox_length = 0;

  if (argc > 2 || (argc == 2 && argv[1][0] == '-')) {
#ifdef USE_HILDON
    printf(_("usage"), argv[0]);
#else
    printf("Invalid command line parameters.\n"
           "Usage: %s [filename.xoj]\n", argv[0]);
#endif
    gtk_exit(0);
  }
   
  undo = NULL; redo = NULL;
  journal.pages = NULL;
  bgpdf.status = STATUS_NOT_INIT;

  new_journal();  
  
  ui.cur_item_type = ITEM_NONE;
  ui.cur_item = NULL;
  ui.cur_path.coords = NULL;
  ui.cur_path_storage_alloc = 0;
  ui.cur_path.ref_count = 1;
  ui.cur_widths = NULL;
  ui.cur_widths_storage_alloc = 0;

  ui.selection = NULL;
  ui.cursor = NULL;

  ui.cur_brush = &(ui.brushes[0][ui.toolno[0]]);
  for (j=0; j<=NUM_BUTTONS; j++)
    for (i=0; i < NUM_STROKE_TOOLS; i++) {
      b = &(ui.brushes[j][i]);
      b->tool_type = i;
      if (b->color_no>=0) {
        b->color_rgba = predef_colors_rgba[b->color_no];
	if (i == TOOL_HIGHLIGHTER) {
	  b->color_rgba &= ui.hiliter_alpha_mask;
	}
      }
      b->thickness = predef_thickness[i][b->thickness_no];
    }
  for (i=0; i<NUM_STROKE_TOOLS; i++)
    g_memmove(ui.default_brushes+i, &(ui.brushes[0][i]), sizeof(struct Brush));

  ui.cur_mapping = 0;
  ui.which_unswitch_button = 0;
  
  reset_recognizer();

  // initialize various interface elements
  
  gtk_window_set_default_size(GTK_WINDOW (winMain), ui.window_default_width, ui.window_default_height);
  if (ui.maximize_at_start) gtk_window_maximize(GTK_WINDOW (winMain));
  update_toolbar_and_menu();
  update_font_button();

#ifndef USE_HILDON
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("journalApplyAllPages")), ui.bg_apply_all_pages);
  if (ui.fullscreen) {
    gtk_check_menu_item_set_active(
      GTK_CHECK_MENU_ITEM(GET_COMPONENT("viewFullscreen")), TRUE);
    gtk_toggle_tool_button_set_active(
      GTK_TOGGLE_TOOL_BUTTON(GET_COMPONENT("buttonFullscreen")), TRUE);
    gtk_window_fullscreen(GTK_WINDOW(winMain));
  }
  gtk_buttons_set_relief(GTK_BUTTON(GET_COMPONENT("buttonColorChooser")), GTK_RELIEF_NONE);

  allow_all_accels();
  add_scroll_bindings();

  // prevent interface items from stealing focus
  // glade doesn't properly handle can_focus, so manually set it
  gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(GET_COMPONENT("comboLayer")), FALSE);
  g_signal_connect(GET_COMPONENT("spinPageNo"), "activate",
          G_CALLBACK(handle_activate_signal), NULL);
  gtk_container_forall(GTK_CONTAINER(winMain), unset_flags, (gpointer)GTK_CAN_FOCUS);
  GTK_WIDGET_SET_FLAGS(GTK_WIDGET(canvas), GTK_CAN_FOCUS);
  GTK_WIDGET_SET_FLAGS(GTK_WIDGET(GET_COMPONENT("spinPageNo")), GTK_CAN_FOCUS);
  
  // install hooks on button/key/activation events to make the spinPageNo lose focus
  gtk_container_forall(GTK_CONTAINER(winMain), install_focus_hooks, NULL);

  // set up and initialize the canvas

#endif

  w = GET_COMPONENT("scrolledwindowMain");
  gtk_container_add (GTK_CONTAINER (w), GTK_WIDGET (canvas));
#ifdef USE_HILDON
  /* we use the hand tool to scroll around */
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (w), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
#else
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (w), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#endif
  gtk_widget_set_events (GTK_WIDGET (canvas), GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
  gtk_widget_show (GTK_WIDGET (canvas));
  gnome_canvas_set_pixels_per_unit (canvas, ui.zoom);
  gnome_canvas_set_center_scroll_region (canvas, TRUE);
  gtk_layout_get_hadjustment(GTK_LAYOUT (canvas))->step_increment = ui.scrollbar_step_increment;
  gtk_layout_get_vadjustment(GTK_LAYOUT (canvas))->step_increment = ui.scrollbar_step_increment;

  // set up the page size and canvas size
  update_page_stuff();

  g_signal_connect ((gpointer) canvas, "button_press_event",
                    G_CALLBACK (on_canvas_button_press_event),
                    NULL);
  g_signal_connect ((gpointer) canvas, "button_release_event",
                    G_CALLBACK (on_canvas_button_release_event),
                    NULL);
  g_signal_connect ((gpointer) canvas, "enter_notify_event",
                    G_CALLBACK (on_canvas_enter_notify_event),
                    NULL);
  g_signal_connect ((gpointer) canvas, "leave_notify_event",
                    G_CALLBACK (on_canvas_leave_notify_event),
                    NULL);
  g_signal_connect ((gpointer) canvas, "expose_event",
                    G_CALLBACK (on_canvas_expose_event),
                    NULL);
  g_signal_connect ((gpointer) canvas, "key_press_event",
                    G_CALLBACK (on_canvas_key_press_event),
                    winMain);
  g_signal_connect ((gpointer) canvas, "motion_notify_event",
                    G_CALLBACK (on_canvas_motion_notify_event),
                    NULL);
  g_signal_connect ((gpointer) gtk_layout_get_vadjustment(GTK_LAYOUT(canvas)),
                    "value-changed", G_CALLBACK (on_vscroll_changed),
                    NULL);
  g_object_set_data (G_OBJECT (winMain), "canvas", canvas);

  screen = gtk_widget_get_screen(winMain);
  ui.screen_width = gdk_screen_get_width(screen);
  ui.screen_height = gdk_screen_get_height(screen);
  
  can_xinput = FALSE;
  dev_list = gdk_devices_list();
  while (dev_list != NULL) {
    device = (GdkDevice *)dev_list->data;
    if (device != gdk_device_get_core_pointer()) {
      /* get around a GDK bug: map the valuator range CORRECTLY to [0,1] */
#ifdef USE_HILDON
      if (g_ascii_strncasecmp (device->name, "TSC2005 touchscreen", 19) != 0
       		|| g_ascii_strncasecmp (device->name, "TSC2301 touchscreen", 19) != 0) {
	      gtk_widget_set_extension_events (GTK_WIDGET(canvas), GDK_EXTENSION_EVENTS_CURSOR);
#endif	
#ifdef ENABLE_XINPUT_BUGFIX
      gdk_device_set_axis_use(device, 0, GDK_AXIS_IGNORE);
      gdk_device_set_axis_use(device, 1, GDK_AXIS_IGNORE);
#endif
      gdk_device_set_mode(device, GDK_MODE_SCREEN);
#ifdef USE_HILDON
      }
#endif
      if (g_str_has_suffix(device->name, "eraser"))
        gdk_device_set_source(device, GDK_SOURCE_ERASER);
      can_xinput = TRUE;
    }

    dev_list = dev_list->next;
  }
#ifndef USE_HILDON
  if (!can_xinput)
    gtk_widget_set_sensitive(GET_COMPONENT("optionsUseXInput"), FALSE);
#endif

  ui.use_xinput = ui.allow_xinput && can_xinput;

#ifndef USE_HILDON
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsProgressiveBG")), ui.progressive_bg);
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsPrintRuling")), ui.print_ruling);
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsAutoloadPdfXoj")), ui.autoload_pdf_xoj);
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsLeftHanded")), ui.left_handed);
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsShortenMenus")), ui.shorten_menus);
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsAutoSavePrefs")), ui.auto_save_prefs);
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsButtonSwitchMapping")), ui.button_switch_mapping);

  
  hide_unimplemented();
#endif

  update_undo_redo_enabled();
  update_copy_paste_enabled();
#ifndef USE_HILDON
  update_vbox_order(ui.vertical_order[ui.fullscreen?1:0]);
#endif
  gtk_widget_grab_focus(GTK_WIDGET(canvas));

  // show everything...
  
  gtk_widget_show (winMain);
  update_cursor();
  
  /* this will cause extension events to get enabled/disabled, but
     we need the windows to be mapped first */
  gtk_check_menu_item_set_active(
    GTK_CHECK_MENU_ITEM(GET_COMPONENT("optionsUseXInput")), ui.use_xinput);

  /* fix a bug in GTK+ 2.16 and 2.17: scrollbars shouldn't get extended
     input events from pointer motion when cursor moves into main window */

  if (!gtk_check_version(2, 16, 0)) {
    g_signal_connect (
      GET_COMPONENT("menubar"),
      "event", G_CALLBACK (filter_extended_events),
      NULL);
    g_signal_connect (
      GET_COMPONENT("toolbarMain"),
      "event", G_CALLBACK (filter_extended_events),
      NULL);
    g_signal_connect (
      GET_COMPONENT("toolbarPen"),
      "event", G_CALLBACK (filter_extended_events),
      NULL);
    g_signal_connect (
      GET_COMPONENT("statusbar"),
      "event", G_CALLBACK (filter_extended_events),
      NULL);
    g_signal_connect (
      (gpointer)(gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(w))),
      "event", G_CALLBACK (filter_extended_events),
      NULL);
    g_signal_connect (
      (gpointer)(gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(w))),
      "event", G_CALLBACK (filter_extended_events),
      NULL);
  }

  // load the MRU
  
  init_mru();
  
#ifdef USE_HILDON
  ui.filename_pdf = NULL;
  ui.png_files_list = NULL;

  // check if there's an auto-saved file and ask to load it
  autosave_filename = g_build_filename (HILDON_AUTOSAVE_DIR, HILDON_AUTOSAVE_FILENAME, NULL);

  f = gzopen (autosave_filename, "r");
  if (f!=NULL) {
    gchar *old_default_path = ui.default_path;

    g_print ("Opening autosaved file... old_default_path %s\n", old_default_path);

    // there's an auto-saved file
    hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 1);
    open_journal (autosave_filename);
    hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 0);

    g_print ("ui.default_path = %s\n", ui.default_path);
    // open_journal would have set this to HILDON_AUTOSAVE_DIR.
    // Nope, that's not what I want
    ui.default_path = old_default_path;
    g_print ("ui.default_path = %s\n", ui.default_path);
    ui.filename = NULL;
    ui.saved = FALSE;
    set_cursor_busy(FALSE);
  }
#endif

  // and finally, open a file specified on the command line
  // (moved here because display parameters weren't initialized yet...)
  
  if (argc == 1) return;
  set_cursor_busy(TRUE);
  if (g_path_is_absolute(argv[1]))
    tmpfn = g_strdup(argv[1]);
  else {
    tmppath = g_get_current_dir();
    tmpfn = g_build_filename(tmppath, argv[1], NULL);
    g_free(tmppath);
  }
#ifdef USE_HILDON
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 1);
#endif
  success = open_journal(tmpfn);
#ifdef USE_HILDON
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 0);
#endif
  g_free(tmpfn);
  set_cursor_busy(FALSE);
  if (!success) {
#ifdef USE_HILDON
    gchar descr[strlen (_("Error opening file ''"))+strlen (argv[1])];
    sprintf (descr, _("Error opening file '%s'"), argv[1]);
    w = hildon_note_new_information (GTK_WINDOW (winMain), descr);
#else
    w = gtk_message_dialog_new(GTK_WINDOW (winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
       GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error opening file '%s'"), argv[1]);
#endif
    gtk_dialog_run(GTK_DIALOG(w));
    gtk_widget_destroy(w);
  }
}


int
main (int argc, char *argv[])
{
  gchar *path, *path1, *path2;
  gchar *tmppath;
 
#ifdef USE_HILDON
  osso_return_t osso_ret;
  DBusConnection *sys_conn;

  /* Set locale */
  setlocale (LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Initialize LibOSSO */
  ctx = osso_initialize (PACKAGE, VERSION, 0, NULL);
  g_assert (ctx != NULL);

  /* Register callback for MIME handling */
  osso_ret = osso_mime_set_cb (ctx, osso_mime_callback, NULL);
  if (osso_ret != OSSO_OK) {
	  fprintf (stderr, "Failed to set MIME callback function!\n");

	  return OSSO_ERROR;
  }

  /* Register to handle hardware events */
  osso_ret = osso_hw_set_event_cb (ctx, NULL, on_hardwareEvent_handler,
		  NULL);
  if (osso_ret != OSSO_OK) {
	  fprintf (stderr, "Failed to set HW state callback function!\n");

	  return OSSO_ERROR;
  }

  osso_ret = osso_application_set_autosave_cb (ctx, on_ossoAutosave_handler,
		  NULL);
  if (osso_ret != OSSO_OK) {
	  fprintf (stderr,"Failed to set Auto Save callback function!\n");

	  return OSSO_ERROR;
  }
  
#else
#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif
#endif

  gtk_set_locale ();

#ifdef USE_HILDON
  hildon_gtk_init (&argc, &argv);
  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps/" PACKAGE);
  theme_name = calloc (1024,1);
  grab_volume_keys = 1;
#else
  gtk_init (&argc, &argv);
  add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");
#endif

  path = g_path_get_dirname(argv[0]);
  path1 = g_build_filename(path, "pixmaps", NULL);
  path2 = g_build_filename(path, "..", "pixmaps", NULL);
  add_pixmap_directory (path1);
  add_pixmap_directory (path2);
  add_pixmap_directory (path);
  g_free(path);
  g_free(path1);
  g_free(path2);

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */

#ifdef USE_HILDON
  // create some data structures needed to populate the preferences
  ui.default_page.bg = g_new(struct Background, 1);

  // initialize config file names
  tmppath = g_build_filename(PACKAGE_DATA_DIR, CONFIG_DIR, NULL);
  mkdir(tmppath, 0700); // safer (MRU data may be confidential)
  ui.mrufile = g_build_filename(tmppath, MRU_FILE, NULL);
  ui.configfile = g_build_filename(tmppath, CONFIG_FILE, NULL);
  g_free(tmppath);

  // make sure autosave dir exists
  if (!mkdir (HILDON_AUTOSAVE_DIR, 0700)) {
	  autosave_disabled = FALSE;
  } else {
	  if (errno == EEXIST) {
		  // well I should ckeck if it is a directory
		  autosave_disabled = FALSE;
	  } else {
		  autosave_disabled = TRUE;
	  }
  }

  // initialize preferences
  init_config_default();
  load_config_from_file();
  //
  // listen to rotation
  // enable accelerator
  if (osso_rpc_run_system(ctx, MCE_SERVICE, MCE_REQUEST_PATH,
          MCE_REQUEST_IF, MCE_ACCELEROMETER_ENABLE_REQ, NULL, DBUS_TYPE_INVALID) == OSSO_OK) {
	g_printerr("INFO: Accelerometers enabled\n");
  } else {
        g_printerr("WARN: Cannot enable accelerometers\n");
  }

  sys_conn = osso_get_sys_dbus_connection(ctx);
  if (sys_conn) {
	DBusError error;
	dbus_error_init (&error);
	dbus_bus_add_match (sys_conn, MCE_SIGNAL_MATCH, &error);
	if (dbus_error_is_set(&error)){
		printf("dbus_bus_add_match failed: %s\n", error.message);
		dbus_error_free(&error);
	}

	if (!dbus_connection_add_filter (sys_conn,
		  (DBusHandleMessageFunction) mce_filter_func, NULL, NULL)) {
		printf("Error dbus_connection_add_filter failed\n");
	} else {
		printf ("DBUS filter added\n");
	}
  }

  winMain = create_winMain (ctx);

#else
  winMain = create_winMain ();
#endif

  init_stuff (argc, argv);
  
#ifndef USE_HILDON
  gtk_window_set_icon(GTK_WINDOW(winMain), create_pixbuf("xournal.png"));
#endif
  
#ifdef USE_HILDON
  hildon_grab_volume_keys (grab_volume_keys);
#endif

  gtk_main ();

#ifdef USE_HILDON
  if (osso_rpc_run_system(ctx, MCE_SERVICE, MCE_REQUEST_PATH,
          MCE_REQUEST_IF, MCE_ACCELEROMETER_DISABLE_REQ, NULL, DBUS_TYPE_INVALID) == OSSO_OK) {
	g_printerr("INFO: Accelerometers disable\n");
  } else {
        g_printerr("WARN: Cannot disable accelerometers\n");
  }

  /* Clean hildon stuff */
  osso_mime_unset_cb (ctx);
  osso_application_unset_autosave_cb (ctx, on_ossoAutosave_handler,
		  NULL);
  osso_deinitialize (ctx);
#endif
  
  if (bgpdf.status != STATUS_NOT_INIT) shutdown_bgpdf();

  save_mru_list();
  if (ui.auto_save_prefs) save_config_to_file();
  
  return 0;
}

