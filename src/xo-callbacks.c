#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libgnomecanvas/libgnomecanvas.h>
#include <time.h>
#ifdef USE_HILDON
#include <hildon/hildon.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-color-chooser.h>
#include <hildon/hildon-color-chooser-dialog.h>
#include <hildon/hildon-color-button.h>
#include <hildon/hildon-file-chooser-dialog.h>
#include <hildon/hildon-file-details-dialog.h>
#include <hildon/hildon-note.h>

#define HILDON_LIBSHARINGDIALOG_LIB "libsharingdialog.so.0"
#include <dlfcn.h>

/*
#include <hildon/hildon-help.h>
*/
#include <hildon/hildon-banner.h>
/* send via email: */
#include <libmodest-dbus-client/libmodest-dbus-client.h>
/* send via bt: */
#include <conbtdialogs-dbus.h>
#include <dbus/dbus-glib.h>
#define _(String) gettext (String)
#endif
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

#include "xournal.h"
#include "xo-callbacks.h"
#include "xo-interface.h"
#include "xo-support.h"
#include "xo-misc.h"
#include "xo-file.h"
#include "xo-paint.h"
#ifndef USE_HILDON
#include "xo-print.h"
#endif
#include "xo-shapes.h"

#ifdef USE_HILDON
#include "xo-hildon_ui.h"
#include "hildon-extras/he-about-dialog.h"

extern GtkWidget *tools_view, *settings_view, *journal_view;

/* Maemo specific handlers */
void
hildon_about (GtkAction *action, gpointer user_data)
{
	HeAboutDialog *about = HE_ABOUT_DIALOG(he_about_dialog_new ());

	gtk_window_set_transient_for (GTK_WINDOW(about), GTK_WINDOW (winMain));

	he_about_dialog_set_app_name (about, "Xournal");
	he_about_dialog_set_bugtracker (about, "https://bugs.maemo.org/enter_bug.cgi?product=Xournal");
	he_about_dialog_set_copyright (about, "Aniello Del Sorbo");
	he_about_dialog_set_description (about, "A note taking / PDF annotation application");
	he_about_dialog_set_icon_name (about, "xournal");
	he_about_dialog_set_version (about, VERSION);
	he_about_dialog_set_website (about, "http://anidel.blogspot.com");
	he_about_dialog_set_donate_url (about, "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=G55CCBR3CNKEC&lc=GB&item_name=MaemoXournal&currency_code=GBP");

	gtk_dialog_run (GTK_DIALOG (about));
	gtk_widget_destroy (GTK_WIDGET (about));
}

void
on_HildonHelp_activate (GtkWidget *widget, gchar *help_id)
{
#ifdef ZERO
  osso_return_t retval;

  if (!help_id)
    return;

  /* Check if "help_id" exists */
  retval = hildon_help_show(ctx,     /* global osso_context */
	  help_id, 		             /* topic id */
	  HILDON_HELP_SHOW_JUSTASK);

  if (retval == 0) {
    hildon_help_show (ctx, help_id, 0);
  }
#endif
}

void
on_HildonShowLayer_activate (GtkWidget *widget, gpointer data)
{
  GtkWidget *old_selector;
  GtkWidget *selector;
  gchar *label;
  gint i;

  old_selector = hildon_picker_button_get_selector (HILDON_PICKER_BUTTON (pickerButton));
  // TODO: destroy it

  selector = hildon_touch_selector_new_text ();

  label = g_strdup_printf (_("Add new layer"));
  hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR(selector),
		label);
  g_free (label);

  label = g_strdup_printf (_("Background"));
  hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR(selector),
		label);
  g_free (label);

  for (i=0; i<ui.cur_page->nlayers;i++) {
    label = g_strdup_printf (_("Layer %d"), i);
    hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR(selector),
		label);
    g_free (label);
  }

  hildon_touch_selector_set_column_selection_mode (
		HILDON_TOUCH_SELECTOR(selector),
		HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE);

  hildon_picker_button_set_selector (HILDON_PICKER_BUTTON(pickerButton),
		HILDON_TOUCH_SELECTOR(selector));

  g_signal_connect (G_OBJECT (selector), "changed",
		G_CALLBACK (on_comboLayer_changed), (gpointer) "maemo");

  printf ("Showing\n");
  gtk_widget_show_all (HILDON_BUTTON(selector));
}

void
on_colorButton_activate (GtkWidget *widget, gpointer data)
{
  GdkColor color = {0, 0, 0, 0};
  GtkWidget *selector;
  gint result;

  selector = HILDON_COLOR_CHOOSER_DIALOG (widget);
/*  g_object_get (widget, "color", &color, NULL);*/
 /*
  selector = hildon_color_chooser_dialog_new();
  hildon_color_chooser_dialog_set_color(
		  HILDON_COLOR_CHOOSER_DIALOG(selector),
		  &color);
  g_print ("Running gtk_dialog_run\n");
  result = gtk_dialog_run(GTK_DIALOG(selector));
  switch (result) {
	  case GTK_RESPONSE_OK:
*/
  		  g_print ("Getting color\n");
		  hildon_color_button_get_color
			  (HILDON_COLOR_BUTTON(buttonColor), &color);
		  
  		  g_print ("activating\n");
		  hildon_process_color_activate (widget, &color);

/*
		  break;
  }
*/
  
  gtk_widget_destroy(selector);
}

void osso_mime_callback (gpointer data, int argc, gchar **argv)
{
  GtkWidget *dialog;
  GtkFileFilter *filt_all, *filt_xoj;
  char *filename;
  gchar *descr;
  gboolean success;

  end_text();
  if (!ok_to_close()) return; // user aborted on save confirmation
    
  filename = g_filename_from_uri(argv[argc-1], NULL, NULL);
    
  set_cursor_busy(TRUE);
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 1);
  success = open_journal(filename);
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 0);
  set_cursor_busy(FALSE);
    
  if (success) { g_free(filename); return; }
    
  /* open failed */

  descr = (char *)malloc (strlen (_("Error opening file ''"))+strlen(g_basename(filename)));

  sprintf (descr, ("Error opening file '%s'"), g_basename(filename));
  dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  g_free(filename);
  free (descr);
}

void
hildon_handle_saveunsaved (gpointer data)
{
  gchar *tmp_filename;
  gboolean tmp_uisaved;

  if (autosave_disabled) {
	  return;
  }

  /*
  hildon_banner_show_information (GTK_WIDGET(winMain),
		NULL, _("Autosaving document..."));
  */

  tmp_filename = ui.filename;
  tmp_uisaved = ui.saved;
  ui.saved = FALSE;
  ui.filename = g_build_filename(HILDON_AUTOSAVE_DIR, HILDON_AUTOSAVE_FILENAME, NULL);
  on_fileSave_activate (NULL, NULL);
  ui.filename = tmp_filename;
  ui.saved = tmp_uisaved;
}

void
hildon_handle_shutdown (gpointer data)
{
  hildon_handle_saveunsaved (data);
}

void
hildon_handle_inactivity (gpointer data)
{
  hildon_handle_saveunsaved (data);
}

void
hildon_handle_memorylow (gpointer data)
{
  if (ui.saved != TRUE) {
    hildon_banner_show_information (GTK_WIDGET (winMain),
		NULL, _("Memory low! Close unused applications and save document"));
  } else {
    hildon_banner_show_information (GTK_WIDGET (winMain),
		NULL, _("Memory low! Close unused applications to free memory"));
  }
}

void
on_hardwareEvent_handler (osso_hw_state_t *state, gpointer data)
{
  if (state->shutdown_ind) {
    hildon_handle_shutdown (data);
  }

  if (state->memory_low_ind) {
    hildon_handle_memorylow (data);
  }

  if (state->save_unsaved_data_ind) {
    hildon_handle_saveunsaved (data);
  }

  if (state->system_inactivity_ind) {
    hildon_handle_inactivity (data);
  }
}

void
on_exitDbus_handler (gboolean die_now, gpointer data)
{
  hildon_handle_saveunsaved (data);
}

void
on_ossoAutosave_handler (gpointer data)
{
  hildon_handle_saveunsaved (data);
}
#endif

void
on_fileNew_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  if (close_journal()) {
    new_journal();
    ui.zoom = ui.startup_zoom;
    update_page_stuff();
    gtk_adjustment_set_value(gtk_layout_get_vadjustment(GTK_LAYOUT(canvas)), 0);
    gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
  }
}


void
on_fileNewBackground_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog, *attach_opt;
  GtkFileFilter *filt_all, *filt_pdf;
  char *filename;
  int file_domain;
  gboolean success;
#ifdef USE_HILDON
  HildonFileSystemModel *model = NULL;

  if (menuitem != NULL) {
#endif
  
  end_text();
  if (!ok_to_close()) return; // user aborted on save confirmation

#ifdef USE_HILDON
  model = HILDON_FILE_SYSTEM_MODEL(g_object_new(HILDON_TYPE_FILE_SYSTEM_MODEL,
			  "ref_widget", GTK_WIDGET(winMain), NULL));

  dialog = hildon_file_chooser_dialog_new_with_properties (
		  GTK_WINDOW(winMain),
		  "file_system_model", model,
		  "action", GTK_FILE_CHOOSER_ACTION_OPEN,
		  NULL);
#else
  dialog = gtk_file_chooser_dialog_new(_("Open PDF"), GTK_WINDOW (winMain),
     GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
#endif
     
  filt_all = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_all, _("All files"));
  gtk_file_filter_add_pattern(filt_all, "*");
  filt_pdf = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_pdf, _("PDF files"));
  gtk_file_filter_add_pattern(filt_pdf, "*.pdf");
  gtk_file_filter_add_pattern(filt_pdf, "*.PDF");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_pdf);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_all);

  if (ui.default_path!=NULL) gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), ui.default_path);

  attach_opt = gtk_check_button_new_with_label(_("Attach file to the journal"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attach_opt), FALSE);
  gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER (dialog), attach_opt);
  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    return;
  }
  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

  gtk_widget_destroy(dialog);

#ifdef USE_HILDON
  } else {
    filename = (char *)user_data;
  }

  file_domain = DOMAIN_ATTACH;
#else
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attach_opt)))
       file_domain = DOMAIN_ATTACH;
  else file_domain = DOMAIN_ABSOLUTE;
#endif
  
  set_cursor_busy(TRUE);
  ui.saved = TRUE; // force close_journal to work
  close_journal();
  while (bgpdf.status != STATUS_NOT_INIT) {
    // waiting for pdf processes to finish dying
    gtk_main_iteration(); 
  }
  new_journal();
  ui.zoom = ui.startup_zoom;
  gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
  update_page_stuff();
  success = init_bgpdf(filename, TRUE, file_domain);
  set_cursor_busy(FALSE);
  if (success) {
    if (menuitem != NULL)
      g_free(filename);
    return;
  }
  
  /* open failed */
#ifdef USE_HILDON
  {
    gchar descr[strlen(_("Error opening file ''"))+ strlen(g_basename(filename))];
    sprintf (descr, _("Error opening file '%s'"), g_basename(filename));
    dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
#else
  dialog = gtk_message_dialog_new(GTK_WINDOW (winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error opening file '%s'"), filename);
#endif
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
#ifdef USE_HILDON
  }
#endif
  g_free(filename);
}


void
on_fileOpen_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog;
  GtkFileFilter *filt_all, *filt_xoj;
  char *filename;
  gboolean success;
#ifdef USE_HILDON
  HildonFileSystemModel *model = NULL;
#endif
  
  end_text();
  if (!ok_to_close()) return; // user aborted on save confirmation
  
#ifdef USE_HILDON
  model = HILDON_FILE_SYSTEM_MODEL(g_object_new(HILDON_TYPE_FILE_SYSTEM_MODEL,
			  "ref_widget", GTK_WIDGET(winMain), NULL));

  dialog = hildon_file_chooser_dialog_new_with_properties (
		  GTK_WINDOW(winMain),
		  "file_system_model", model,
		  "action", GTK_FILE_CHOOSER_ACTION_OPEN,
		  NULL);
#else
  dialog = gtk_file_chooser_dialog_new(_("Open Journal"), GTK_WINDOW (winMain),
     GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
#ifdef FILE_DIALOG_SIZE_BUGFIX
  gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
#endif
#endif
     
  filt_all = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_all, _("All files"));
  gtk_file_filter_add_pattern(filt_all, "*");
  filt_xoj = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_xoj, _("Xournal files"));
  gtk_file_filter_add_pattern(filt_xoj, "*.xoj");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_xoj);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_all);

  if (ui.default_path!=NULL) gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), ui.default_path);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    return;
  }
  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
  gtk_widget_destroy(dialog);

#ifdef USE_HILDON
  if (filename) {
	  gchar extension[3];

	  strncpy (extension, filename+strlen(filename)-3, 3);
	  if (strncmp (extension, "pdf", 3) == 0) {
		  on_fileNewBackground_activate (NULL, filename);
		  g_free (filename);

		  return;
	  }
  }

  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 1);
#else
  set_cursor_busy(TRUE);
#endif
  success = open_journal(filename);
#ifdef USE_HILDON
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 0);
#else
  set_cursor_busy(FALSE);
#endif
  if (success) { g_free(filename); return; }
  
  /* open failed */
#ifdef USE_HILDON
  {
    gchar descr[strlen(_("Error opening file ''"))+strlen (g_basename(filename))];
    sprintf (descr, _("Error opening file '%s'"), g_basename(filename));
    dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
  }
#else
  dialog = gtk_message_dialog_new(GTK_WINDOW (winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error opening file '%s'"), filename);
#endif
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  g_free(filename);

}


void
on_fileSave_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog;
  
  end_text();
  if (ui.filename == NULL) {
    on_fileSaveAs_activate(menuitem, user_data);
    return;
  }
  set_cursor_busy(TRUE);
  if (save_journal(ui.filename)) { // success
    set_cursor_busy(FALSE);
    ui.saved = TRUE;
    return;
  }
  set_cursor_busy(FALSE);
  /* save failed */
#ifdef USE_HILDON
  {
    gchar descr[strlen(_("Error saving file ''"))+strlen (g_basename(ui.filename))];
    sprintf (descr, _("Error saving file '%s'"), g_basename(ui.filename));
    dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
  }
#else
  dialog = gtk_message_dialog_new(GTK_WINDOW (winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error saving file '%s'"), ui.filename);
#endif
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}


void
on_fileSaveAs_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog, *warning_dialog;
  GtkFileFilter *filt_all, *filt_xoj;
  char *filename;
  char stime[30];
  time_t curtime;
  gboolean warn;
  struct stat stat_buf;
#ifdef USE_HILDON
  HildonFileSystemModel *model = NULL;
#endif
  
  end_text();

#ifdef USE_HILDON
  model = HILDON_FILE_SYSTEM_MODEL(g_object_new(HILDON_TYPE_FILE_SYSTEM_MODEL,
			  "ref_widget", GTK_WIDGET(winMain), NULL));

  dialog = hildon_file_chooser_dialog_new_with_properties (
		  GTK_WINDOW(winMain),
		  "file_system_model", model,
		  "action", GTK_FILE_CHOOSER_ACTION_SAVE,
		  NULL);
#else
  dialog = gtk_file_chooser_dialog_new(_("Save Journal"), GTK_WINDOW (winMain),
     GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
#ifdef FILE_DIALOG_SIZE_BUGFIX
  gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
#endif
#endif
     
  if (ui.filename!=NULL) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), ui.filename);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), g_basename(ui.filename));
  } 
  else
  if (bgpdf.status!=STATUS_NOT_INIT && bgpdf.file_domain == DOMAIN_ABSOLUTE 
      && bgpdf.filename != NULL) {
    filename = g_strdup_printf("%s.xoj", bgpdf.filename->s);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), filename);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), g_basename(filename));
    g_free(filename); 
  }
  else {
    curtime = time(NULL);
    strftime(stime, 30, "%F-Note-%H-%M.xoj", localtime(&curtime));
    if (ui.default_path!=NULL)
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), ui.default_path);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), stime);
  }
     
  filt_all = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_all, _("All files"));
  gtk_file_filter_add_pattern(filt_all, "*");
  filt_xoj = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_xoj, _("Xournal files"));
  gtk_file_filter_add_pattern(filt_xoj, "*.xoj");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_xoj);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_all);
  
  // somehow this doesn't seem to be set by default
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

  do {
    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
      gtk_widget_destroy(dialog);
      return;
    }
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    warn = g_file_test (filename, G_FILE_TEST_EXISTS);
    if (warn) { // ok to overwrite an empty file
      if (!g_stat(filename, &stat_buf))
        if (stat_buf.st_size == 0) warn=FALSE;
    }
    if (warn && ui.filename!=NULL) { // ok to overwrite oneself
      if (ui.filename[0]=='/' && !strcmp(ui.filename, filename)) warn=FALSE;
      if (ui.filename[0]!='/' && g_str_has_suffix(filename, ui.filename)) warn=FALSE;
    }
    if (warn) {
#ifdef USE_HILDON
      gchar descr[strlen(_("Should the file '' be overwritten?"))+ strlen(g_basename(filename))];
      sprintf (descr, _("Should the file '%s' be overwritten?"), g_basename(filename));
      warning_dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
      hildon_note_set_button_texts (HILDON_NOTE(warning_dialog), _("Yes"), _("No"));
      if (gtk_dialog_run(GTK_DIALOG(warning_dialog)) == GTK_RESPONSE_OK)
#else
      warning_dialog = gtk_message_dialog_new(GTK_WINDOW(winMain), 
        GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        _("Should the file %s be overwritten?"), filename);
      if (gtk_dialog_run(GTK_DIALOG(warning_dialog)) == GTK_RESPONSE_YES)
#endif
        warn = FALSE;
      gtk_widget_destroy(warning_dialog);
    }
  } while (warn);

  gtk_widget_destroy(dialog);

  set_cursor_busy(TRUE);
  if (save_journal(filename)) { // success
    ui.saved = TRUE;
    set_cursor_busy(FALSE);
    update_file_name(filename);
    return;
  }
  set_cursor_busy(FALSE);
  /* save failed */
#ifdef USE_HILDON
  {
    gchar descr[strlen(_("Error saving file ''"))+strlen (g_basename(filename))];
    sprintf (descr, _("Error saving file '%s'"), g_basename(filename));
    dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
  }
#else
  dialog = gtk_message_dialog_new(GTK_WINDOW (winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error saving file '%s'"), filename);
#endif
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  g_free(filename);
}


void
on_filePrintOptions_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_filePrint_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	/* Maemo does not print */
#ifndef USE_HILDON
#if GTK_CHECK_VERSION(2, 10, 0)
  GtkPrintOperation *print;
  GtkPrintOperationResult res;

  int fromPage, toPage;
  int response;
  char *in_fn, *p;

  end_text();

  if (!gtk_check_version(2, 10, 0)) {
    print = gtk_print_operation_new();
/*
    if (!ui.print_settings)
      ui.print_settings = gtk_print_settings_new();
    if (ui.filename!=NULL) {
      if (g_str_has_suffix(ui.filename, ".xoj")) {
        in_fn = g_strdup(ui.filename);
        g_strlcpy(g_strrstr(in_fn, "xoj"), "pdf", 4);
      } 
      else in_fn = g_strdup_printf("%s.pdf", ui.filename);
      gtk_print_settings_set(ui.print_settings, GTK_PRINT_SETTINGS_OUTPUT_URI,
         g_filename_to_uri(in_fn, NULL, NULL));
      g_free(in_fn);
    }
*/

    if (ui.print_settings!=NULL)
       gtk_print_operation_set_print_settings (print, ui.print_settings);
    gtk_print_operation_set_n_pages(print, journal.npages);
    gtk_print_operation_set_current_page(print, ui.pageno);
    gtk_print_operation_set_show_progress(print, TRUE);
    if (ui.filename!=NULL) {
      p = g_utf8_strrchr(ui.filename, -1, '/');
      if (p==NULL) p = ui.filename;
      else p++;
      gtk_print_operation_set_job_name(print, p);
    }
    g_signal_connect (print, "draw_page", G_CALLBACK (print_job_render_page), NULL);
    res = gtk_print_operation_run(print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                  GTK_WINDOW(winMain), NULL);
    if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
      if (ui.print_settings!=NULL) g_object_unref(ui.print_settings);
      ui.print_settings = g_object_ref(gtk_print_operation_get_print_settings(print));
    }

    g_object_unref(print);
  }
#endif
#endif
}


void
on_filePrintPDF_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog, *warning_dialog;
  GtkFileFilter *filt_all, *filt_pdf;
  char *filename, *in_fn;
  char stime[30];
  time_t curtime;
  gboolean warn;
#ifdef USE_HILDON
  HildonFileSystemModel *model = NULL;
#endif
  
  end_text();
#ifdef USE_HILDON
  model = HILDON_FILE_SYSTEM_MODEL(g_object_new(HILDON_TYPE_FILE_SYSTEM_MODEL,
			  "ref_widget", GTK_WIDGET(winMain), NULL));

  dialog = hildon_file_chooser_dialog_new (
		  GTK_WINDOW(winMain),
		  GTK_FILE_CHOOSER_ACTION_SAVE);
#else
  dialog = gtk_file_chooser_dialog_new(_("Export to PDF"), GTK_WINDOW (winMain),
     GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
#ifdef FILE_DIALOG_SIZE_BUGFIX
  gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
#endif
#endif
     
  if (ui.filename!=NULL) {
    if (g_str_has_suffix(ui.filename, ".xoj")) {
      in_fn = g_strdup(ui.filename);
      g_strlcpy(g_strrstr(in_fn, "xoj"), "pdf", 4);
    } 
    else
      in_fn = g_strdup_printf("%s.pdf", ui.filename);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), in_fn);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), g_basename(in_fn));
  } else {
    curtime = time(NULL);
    strftime(stime, 30, "%F-Note-%H-%M.pdf", localtime(&curtime));

    if (ui.default_path!=NULL)
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), ui.default_path);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), stime);
    in_fn = NULL;
  }
     
  filt_all = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_all, _("All files"));
  gtk_file_filter_add_pattern(filt_all, "*");
  filt_pdf = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_pdf, _("PDF files"));
  gtk_file_filter_add_pattern(filt_pdf, "*.pdf");
  gtk_file_filter_add_pattern(filt_pdf, "*.PDF");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_pdf);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_all);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  g_free(in_fn);
  
  do {
    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
      gtk_widget_destroy(dialog);
      if (!ui.filename_pdf)
        g_free (ui.filename_pdf);
      ui.filename_pdf = NULL;
      return;
    }
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    if (!filename) {
      if (!ui.filename_pdf)
        g_free (ui.filename_pdf);
      ui.filename_pdf = NULL;
      return;
    }

    warn = g_file_test(filename, G_FILE_TEST_EXISTS);
    if (warn) {
#ifdef USE_HILDON
      gchar descr[strlen(_("Should the file '' be overwritten?"))+ strlen(g_basename(filename))];
      sprintf (descr, _("Should the file '%s' be overwritten?"), g_basename(filename));
      warning_dialog = hildon_note_new_confirmation (GTK_WINDOW (winMain), descr);
      hildon_note_set_button_texts (HILDON_NOTE(warning_dialog), _("Yes"), _("No"));
      if (gtk_dialog_run(GTK_DIALOG(warning_dialog)) == GTK_RESPONSE_OK)
#else
      warning_dialog = gtk_message_dialog_new(GTK_WINDOW(winMain),
        GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        _("Should the file %s be overwritten?"), filename);
      if (gtk_dialog_run(GTK_DIALOG(warning_dialog)) == GTK_RESPONSE_YES)
#endif
        warn = FALSE;
      gtk_widget_destroy(warning_dialog);
    }
  } while(warn);
    
  gtk_widget_destroy(dialog);

  set_cursor_busy(TRUE);
  if (!print_to_pdf(filename)) {
#ifdef USE_HILDON
    gchar descr[strlen(_("Error creating file ''"))+ strlen(g_basename(filename))];
    sprintf (descr, _("Error creating file '%s'"), g_basename(filename));
    warning_dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
#endif
    set_cursor_busy(FALSE);
#ifndef USE_HILDON
    dialog = gtk_message_dialog_new(GTK_WINDOW (winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error creating file '%s'"), filename);
#endif
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
  set_cursor_busy(FALSE);

#ifdef USE_HILDON
  if (ui.filename_pdf != NULL)
     g_free (ui.filename_pdf);
  ui.filename_pdf = filename;
#else
  g_free(filename);
#endif
}


void
on_fileQuit_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  if (ok_to_close()) gtk_main_quit ();
}


void
on_editUndo_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct UndoItem *u;
  GList *list, *itemlist;
  struct UndoErasureData *erasure;
  struct Item *it;
  struct Brush tmp_brush;
  struct Background *tmp_bg;
  double tmp_x, tmp_y;
  gchar *tmpstr;
  GnomeCanvasGroup *group;
  
  end_text();
  if (undo == NULL) return; // nothing to undo!
  reset_selection(); // safer
  reset_recognizer(); // safer
  if (undo->type == ITEM_STROKE || undo->type == ITEM_TEXT) {
    // we're keeping the stroke info, but deleting the canvas item
    gtk_object_destroy(GTK_OBJECT(undo->item->canvas_item));
    undo->item->canvas_item = NULL;
    // we also remove the object from its layer!
    undo->layer->items = g_list_remove(undo->layer->items, undo->item);
    undo->layer->nitems--;
  }
  else if (undo->type == ITEM_ERASURE || undo->type == ITEM_RECOGNIZER) {
    for (list = undo->erasurelist; list!=NULL; list = list->next) {
      erasure = (struct UndoErasureData *)list->data;
      // delete all the created items
      for (itemlist = erasure->replacement_items; itemlist!=NULL; itemlist = itemlist->next) {
        it = (struct Item *)itemlist->data;
        gtk_object_destroy(GTK_OBJECT(it->canvas_item));
        it->canvas_item = NULL;
        undo->layer->items = g_list_remove(undo->layer->items, it);
        undo->layer->nitems--;
      }
      // recreate the deleted one
      make_canvas_item_one(undo->layer->group, erasure->item);
      
      undo->layer->items = g_list_insert(undo->layer->items, erasure->item,
                                                             erasure->npos);
      if (erasure->npos == 0)
        lower_canvas_item_to(undo->layer->group, erasure->item->canvas_item, NULL);
      else
        lower_canvas_item_to(undo->layer->group, erasure->item->canvas_item,
          ((struct Item *)g_list_nth_data(undo->layer->items, erasure->npos-1))->canvas_item);
      undo->layer->nitems++;
    }
  }
  else if (undo->type == ITEM_NEW_BG_ONE || undo->type == ITEM_NEW_BG_RESIZE
           || undo->type == ITEM_PAPER_RESIZE) {
    if (undo->type != ITEM_PAPER_RESIZE) {
      // swap the two bg's
      tmp_bg = undo->page->bg;
      undo->page->bg = undo->bg;
      undo->bg = tmp_bg;
      undo->page->bg->canvas_item = undo->bg->canvas_item;
      undo->bg->canvas_item = NULL;
    }
    if (undo->type != ITEM_NEW_BG_ONE) {
      tmp_x = undo->page->width;
      tmp_y = undo->page->height;
      undo->page->width = undo->val_x;
      undo->page->height = undo->val_y;
      undo->val_x = tmp_x;
      undo->val_y = tmp_y;
      make_page_clipbox(undo->page);
    }
    update_canvas_bg(undo->page);
    do_switch_page(g_list_index(journal.pages, undo->page), TRUE, TRUE);
  }
  else if (undo->type == ITEM_NEW_DEFAULT_BG) {
    tmp_bg = ui.default_page.bg;
    ui.default_page.bg = undo->bg;
    undo->bg = tmp_bg;
    tmp_x = ui.default_page.width;
    tmp_y = ui.default_page.height;
    ui.default_page.width = undo->val_x;
    ui.default_page.height = undo->val_y;
    undo->val_x = tmp_x;
    undo->val_y = tmp_y;
  }
  else if (undo->type == ITEM_NEW_PAGE) {
    // unmap the page; keep the page & its empty layer in memory
    if (undo->page->group!=NULL) gtk_object_destroy(GTK_OBJECT(undo->page->group));
      // also destroys the background and layer's canvas items
    undo->page->group = NULL;
    undo->page->bg->canvas_item = NULL;
    journal.pages = g_list_remove(journal.pages, undo->page);
    journal.npages--;
    if (ui.cur_page == undo->page) ui.cur_page = NULL;
        // so do_switch_page() won't try to remap the layers of the defunct page
    if (ui.pageno >= undo->val) ui.pageno--;
    if (ui.pageno < 0) ui.pageno = 0;
    do_switch_page(ui.pageno, TRUE, TRUE);
  }
  else if (undo->type == ITEM_DELETE_PAGE) {
    journal.pages = g_list_insert(journal.pages, undo->page, undo->val);
    journal.npages++;
    make_canvas_items(); // re-create the canvas items
    do_switch_page(undo->val, TRUE, TRUE);
  }
  else if (undo->type == ITEM_MOVESEL) {
    for (itemlist = undo->itemlist; itemlist != NULL; itemlist = itemlist->next) {
      it = (struct Item *)itemlist->data;
      if (it->canvas_item != NULL) {
        if (undo->layer != undo->layer2)
          gnome_canvas_item_reparent(it->canvas_item, undo->layer->group);
        gnome_canvas_item_move(it->canvas_item, -undo->val_x, -undo->val_y);
      }
    }
    move_journal_items_by(undo->itemlist, -undo->val_x, -undo->val_y,
                            undo->layer2, undo->layer, undo->auxlist);
  }
  else if (undo->type == ITEM_RESIZESEL) {
    resize_journal_items_by(undo->itemlist, 
      1/undo->scaling_x, 1/undo->scaling_y,
      -undo->val_x/undo->scaling_x, -undo->val_y/undo->scaling_y);
  }
  else if (undo->type == ITEM_PASTE) {
    for (itemlist = undo->itemlist; itemlist != NULL; itemlist = itemlist->next) {
      it = (struct Item *)itemlist->data;
      gtk_object_destroy(GTK_OBJECT(it->canvas_item));
      it->canvas_item = NULL;
      undo->layer->items = g_list_remove(undo->layer->items, it);
      undo->layer->nitems--;
    }
  }
  else if (undo->type == ITEM_NEW_LAYER) {
    // unmap the layer; keep the empty layer in memory
    if (undo->layer->group!=NULL) gtk_object_destroy(GTK_OBJECT(undo->layer->group));
    undo->layer->group = NULL;
    undo->page->layers = g_list_remove(undo->page->layers, undo->layer);
    undo->page->nlayers--;
    do_switch_page(ui.pageno, FALSE, FALSE); // don't stay with bad cur_layer info
  }
  else if (undo->type == ITEM_DELETE_LAYER) {
    // special case of -1: deleted the last layer, created a new one
    if (undo->val == -1) {
      if (undo->layer2->group!=NULL) gtk_object_destroy(GTK_OBJECT(undo->layer2->group));
      undo->layer2->group = NULL;
      undo->page->layers = g_list_remove(undo->page->layers, undo->layer2);
      undo->page->nlayers--;
    }
    // re-map the layer
    undo->layer->group = (GnomeCanvasGroup *) gnome_canvas_item_new(
      undo->page->group, gnome_canvas_group_get_type(), NULL);
    lower_canvas_item_to(undo->page->group, GNOME_CANVAS_ITEM(undo->layer->group),
      (undo->val >= 1) ? GNOME_CANVAS_ITEM(((struct Layer *)
            g_list_nth_data(undo->page->layers, undo->val-1))->group) :
            undo->page->bg->canvas_item);
    undo->page->layers = g_list_insert(undo->page->layers, undo->layer,
                                     (undo->val >= 0) ? undo->val:0);
    undo->page->nlayers++;
    
    for (itemlist = undo->layer->items; itemlist!=NULL; itemlist = itemlist->next)
      make_canvas_item_one(undo->layer->group, (struct Item *)itemlist->data);

    do_switch_page(ui.pageno, FALSE, FALSE); // show the restored layer & others...
  }
  else if (undo->type == ITEM_REPAINTSEL) {
    for (itemlist = undo->itemlist, list = undo->auxlist; itemlist!=NULL;
           itemlist = itemlist->next, list = list->next) {
      it = (struct Item *)itemlist->data;
      g_memmove(&tmp_brush, &(it->brush), sizeof(struct Brush));
      g_memmove(&(it->brush), list->data, sizeof(struct Brush));
      g_memmove(list->data, &tmp_brush, sizeof(struct Brush));
      if (it->type == ITEM_STROKE && it->canvas_item != NULL) {
        // remark: a variable-width item might have lost its variable-width
        group = (GnomeCanvasGroup *) it->canvas_item->parent;
        gtk_object_destroy(GTK_OBJECT(it->canvas_item));
        make_canvas_item_one(group, it);
      }
      if (it->type == ITEM_TEXT && it->canvas_item != NULL)
        gnome_canvas_item_set(it->canvas_item, 
          "fill-color-rgba", it->brush.color_rgba, NULL);
    }
  }
  else if (undo->type == ITEM_TEXT_EDIT) {
    tmpstr = undo->str;
    undo->str = undo->item->text;
    undo->item->text = tmpstr;
    gnome_canvas_item_set(undo->item->canvas_item, "text", tmpstr, NULL);
    update_item_bbox(undo->item);
  }
  else if (undo->type == ITEM_TEXT_ATTRIB) {
    tmpstr = undo->str;
    undo->str = undo->item->font_name;
    undo->item->font_name = tmpstr;
    tmp_x = undo->val_x;
    undo->val_x = undo->item->font_size;
    undo->item->font_size = tmp_x;
    g_memmove(&tmp_brush, undo->brush, sizeof(struct Brush));
    g_memmove(undo->brush, &(undo->item->brush), sizeof(struct Brush));
    g_memmove(&(undo->item->brush), &tmp_brush, sizeof(struct Brush));
    gnome_canvas_item_set(undo->item->canvas_item, 
      "fill-color-rgba", undo->item->brush.color_rgba, NULL);
    update_text_item_displayfont(undo->item);
    update_item_bbox(undo->item);
  }
  
  // move item from undo to redo stack
  u = undo;
  undo = undo->next;
  u->next = redo;
  redo = u;
  ui.saved = FALSE;
#ifdef USE_HILDON
  osso_application_userdata_changed (ctx);
#endif
  update_undo_redo_enabled();
  if (u->multiop & MULTIOP_CONT_UNDO) on_editUndo_activate(NULL,NULL); // loop
}


void
on_editRedo_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct UndoItem *u;
  GList *list, *itemlist, *target;
  struct UndoErasureData *erasure;
  struct Item *it;
  struct Brush tmp_brush;
  struct Background *tmp_bg;
  struct Layer *l;
  double tmp_x, tmp_y;
  gchar *tmpstr;
  GnomeCanvasGroup *group;
  
  end_text();
  if (redo == NULL) return; // nothing to redo!
  reset_selection(); // safer
  reset_recognizer(); // safer
  if (redo->type == ITEM_STROKE || redo->type == ITEM_TEXT) {
    // re-create the canvas_item
    make_canvas_item_one(redo->layer->group, redo->item);
    // reinsert the item on its layer
    redo->layer->items = g_list_append(redo->layer->items, redo->item);
    redo->layer->nitems++;
  }
  else if (redo->type == ITEM_ERASURE || redo->type == ITEM_RECOGNIZER) {
    for (list = redo->erasurelist; list!=NULL; list = list->next) {
      erasure = (struct UndoErasureData *)list->data;
      target = g_list_find(redo->layer->items, erasure->item);
      // re-create all the created items
      for (itemlist = erasure->replacement_items; itemlist!=NULL; itemlist = itemlist->next) {
        it = (struct Item *)itemlist->data;
        make_canvas_item_one(redo->layer->group, it);
        redo->layer->items = g_list_insert_before(redo->layer->items, target, it);
        redo->layer->nitems++;
        lower_canvas_item_to(redo->layer->group, it->canvas_item, erasure->item->canvas_item);
      }
      // re-delete the deleted one
      gtk_object_destroy(GTK_OBJECT(erasure->item->canvas_item));
      erasure->item->canvas_item = NULL;
      redo->layer->items = g_list_delete_link(redo->layer->items, target);
      redo->layer->nitems--;
    }
  }
  else if (redo->type == ITEM_NEW_BG_ONE || redo->type == ITEM_NEW_BG_RESIZE
           || redo->type == ITEM_PAPER_RESIZE) {
    if (redo->type != ITEM_PAPER_RESIZE) {
      // swap the two bg's
      tmp_bg = redo->page->bg;
      redo->page->bg = redo->bg;
      redo->bg = tmp_bg;
      redo->page->bg->canvas_item = redo->bg->canvas_item;
      redo->bg->canvas_item = NULL;
    }
    if (redo->type != ITEM_NEW_BG_ONE) {
      tmp_x = redo->page->width;
      tmp_y = redo->page->height;
      redo->page->width = redo->val_x;
      redo->page->height = redo->val_y;
      redo->val_x = tmp_x;
      redo->val_y = tmp_y;
      make_page_clipbox(redo->page);
    }
    update_canvas_bg(redo->page);
    do_switch_page(g_list_index(journal.pages, redo->page), TRUE, TRUE);
  }
  else if (redo->type == ITEM_NEW_DEFAULT_BG) {
    tmp_bg = ui.default_page.bg;
    ui.default_page.bg = redo->bg;
    redo->bg = tmp_bg;
    tmp_x = ui.default_page.width;
    tmp_y = ui.default_page.height;
    ui.default_page.width = redo->val_x;
    ui.default_page.height = redo->val_y;
    redo->val_x = tmp_x;
    redo->val_y = tmp_y;
  }
  else if (redo->type == ITEM_NEW_PAGE) {
    // remap the page
    redo->page->bg->canvas_item = NULL;
    redo->page->group = (GnomeCanvasGroup *) gnome_canvas_item_new(
      gnome_canvas_root(canvas), gnome_canvas_clipgroup_get_type(), NULL);
    make_page_clipbox(redo->page);
    update_canvas_bg(redo->page);
    l = (struct Layer *)redo->page->layers->data;
    l->group = (GnomeCanvasGroup *) gnome_canvas_item_new(
      redo->page->group, gnome_canvas_group_get_type(), NULL);
    
    journal.pages = g_list_insert(journal.pages, redo->page, redo->val);
    journal.npages++;
    do_switch_page(redo->val, TRUE, TRUE);
  }
  else if (redo->type == ITEM_DELETE_PAGE) {
    // unmap all the canvas items
    gtk_object_destroy(GTK_OBJECT(redo->page->group));
    redo->page->group = NULL;
    redo->page->bg->canvas_item = NULL;
    for (list = redo->page->layers; list!=NULL; list = list->next) {
      l = (struct Layer *)list->data;
      for (itemlist = l->items; itemlist!=NULL; itemlist = itemlist->next)
        ((struct Item *)itemlist->data)->canvas_item = NULL;
      l->group = NULL;
    }
    journal.pages = g_list_remove(journal.pages, redo->page);
    journal.npages--;
    if (ui.pageno > redo->val || ui.pageno == journal.npages) ui.pageno--;
    ui.cur_page = NULL;
      // so do_switch_page() won't try to remap the layers of the defunct page
    do_switch_page(ui.pageno, TRUE, TRUE);
  }
  else if (redo->type == ITEM_MOVESEL) {
    for (itemlist = redo->itemlist; itemlist != NULL; itemlist = itemlist->next) {
      it = (struct Item *)itemlist->data;
      if (it->canvas_item != NULL) {
        if (redo->layer != redo->layer2)
          gnome_canvas_item_reparent(it->canvas_item, redo->layer2->group);
        gnome_canvas_item_move(it->canvas_item, redo->val_x, redo->val_y);
      }
    }
    move_journal_items_by(redo->itemlist, redo->val_x, redo->val_y,
                            redo->layer, redo->layer2, NULL);
  }
  else if (redo->type == ITEM_RESIZESEL) {
    resize_journal_items_by(redo->itemlist, 
          redo->scaling_x, redo->scaling_y, redo->val_x, redo->val_y);
  }
  else if (redo->type == ITEM_PASTE) {
    for (itemlist = redo->itemlist; itemlist != NULL; itemlist = itemlist->next) {
      it = (struct Item *)itemlist->data;
      make_canvas_item_one(redo->layer->group, it);
      redo->layer->items = g_list_append(redo->layer->items, it);
      redo->layer->nitems++;
    }
  }
  else if (redo->type == ITEM_NEW_LAYER) {
    redo->layer->group = (GnomeCanvasGroup *) gnome_canvas_item_new(
        redo->page->group, gnome_canvas_group_get_type(), NULL);
    lower_canvas_item_to(redo->page->group, GNOME_CANVAS_ITEM(redo->layer->group),
      (redo->val >= 1) ? GNOME_CANVAS_ITEM(((struct Layer *)
            g_list_nth_data(redo->page->layers, redo->val-1))->group) :
            redo->page->bg->canvas_item);
    redo->page->layers = g_list_insert(redo->page->layers, redo->layer, redo->val);
    redo->page->nlayers++;
    do_switch_page(ui.pageno, FALSE, FALSE);
  }
  else if (redo->type == ITEM_DELETE_LAYER) {
    gtk_object_destroy(GTK_OBJECT(redo->layer->group));
    redo->layer->group = NULL;
    for (list=redo->layer->items; list!=NULL; list=list->next)
      ((struct Item *)list->data)->canvas_item = NULL;
    redo->page->layers = g_list_remove(redo->page->layers, redo->layer);
    redo->page->nlayers--;
    if (redo->val == -1) {
      redo->layer2->group = (GnomeCanvasGroup *)gnome_canvas_item_new(
        redo->page->group, gnome_canvas_group_get_type(), NULL);
      redo->page->layers = g_list_append(redo->page->layers, redo->layer2);
      redo->page->nlayers++;
    }
    do_switch_page(ui.pageno, FALSE, FALSE);
  }
  else if (redo->type == ITEM_REPAINTSEL) {
    for (itemlist = redo->itemlist, list = redo->auxlist; itemlist!=NULL;
           itemlist = itemlist->next, list = list->next) {
      it = (struct Item *)itemlist->data;
      g_memmove(&tmp_brush, &(it->brush), sizeof(struct Brush));
      g_memmove(&(it->brush), list->data, sizeof(struct Brush));
      g_memmove(list->data, &tmp_brush, sizeof(struct Brush));
      if (it->type == ITEM_STROKE && it->canvas_item != NULL) {
        // remark: a variable-width item might have lost its variable-width
        group = (GnomeCanvasGroup *) it->canvas_item->parent;
        gtk_object_destroy(GTK_OBJECT(it->canvas_item));
        make_canvas_item_one(group, it);
      }
      if (it->type == ITEM_TEXT && it->canvas_item != NULL)
        gnome_canvas_item_set(it->canvas_item, 
          "fill-color-rgba", it->brush.color_rgba, NULL);
    }
  }
  else if (redo->type == ITEM_TEXT_EDIT) {
    tmpstr = redo->str;
    redo->str = redo->item->text;
    redo->item->text = tmpstr;
    gnome_canvas_item_set(redo->item->canvas_item, "text", tmpstr, NULL);
    update_item_bbox(redo->item);
  }
  else if (redo->type == ITEM_TEXT_ATTRIB) {
    tmpstr = redo->str;
    redo->str = redo->item->font_name;
    redo->item->font_name = tmpstr;
    tmp_x = redo->val_x;
    redo->val_x = redo->item->font_size;
    redo->item->font_size = tmp_x;
    g_memmove(&tmp_brush, redo->brush, sizeof(struct Brush));
    g_memmove(redo->brush, &(redo->item->brush), sizeof(struct Brush));
    g_memmove(&(redo->item->brush), &tmp_brush, sizeof(struct Brush));
    gnome_canvas_item_set(redo->item->canvas_item, 
      "fill-color-rgba", redo->item->brush.color_rgba, NULL);
    update_text_item_displayfont(redo->item);
    update_item_bbox(redo->item);
  }
  
  // move item from redo to undo stack
  u = redo;
  redo = redo->next;
  u->next = undo;
  undo = u;
  ui.saved = FALSE;
#ifdef USE_HILDON
  osso_application_userdata_changed (ctx);
#endif
  update_undo_redo_enabled();
  if (u->multiop & MULTIOP_CONT_REDO) on_editRedo_activate(NULL,NULL); // loop
}


void
on_editCut_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  selection_to_clip();
  selection_delete();
}


void
on_editCopy_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  selection_to_clip();
}


void
on_editPaste_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  clipboard_paste();
}


void
on_editDelete_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  selection_delete();
}


void
on_viewContinuous_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkAdjustment *v_adj;
  double yscroll;
  struct Page *pg;

  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem))) return;
  if (ui.view_continuous) return;
  ui.view_continuous = TRUE;
  v_adj = gtk_layout_get_vadjustment(GTK_LAYOUT(canvas));
  pg = ui.cur_page;
  yscroll = gtk_adjustment_get_value(v_adj) - pg->voffset*ui.zoom;
  update_page_stuff();
  gtk_adjustment_set_value(v_adj, yscroll + pg->voffset*ui.zoom);
  // force a refresh
  gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
}


void
on_viewOnePage_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkAdjustment *v_adj;
  double yscroll;
  
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem))) return;
  if (!ui.view_continuous) return;
  ui.view_continuous = FALSE;
  v_adj = gtk_layout_get_vadjustment(GTK_LAYOUT(canvas));
  yscroll = gtk_adjustment_get_value(v_adj) - ui.cur_page->voffset*ui.zoom;
  update_page_stuff();
  gtk_adjustment_set_value(v_adj, yscroll + ui.cur_page->voffset*ui.zoom);
  // force a refresh
  gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
}


void
on_viewZoomIn_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (ui.zoom > MAX_ZOOM) return;
  ui.zoom *= ui.zoom_step_factor;
  gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
  rescale_text_items();
  rescale_bg_pixmaps();
}


void
on_viewZoomOut_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (ui.zoom < MIN_ZOOM) return;
  ui.zoom /= ui.zoom_step_factor;
  gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
  rescale_text_items();
  rescale_bg_pixmaps();
}


void
on_viewNormalSize_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ui.zoom = DEFAULT_ZOOM;
  gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
  rescale_text_items();
  rescale_bg_pixmaps();
}


void
on_viewPageWidth_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ui.zoom = (GTK_WIDGET(canvas))->allocation.width/ui.cur_page->width;
  gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
  rescale_text_items();
  rescale_bg_pixmaps();
}


void
on_viewFirstPage_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  do_switch_page(0, TRUE, FALSE);
}


void
on_viewPreviousPage_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  if (ui.pageno == 0) return;

  do_switch_page(ui.pageno-1, TRUE, FALSE);
}


void
on_viewNextPage_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  if (ui.pageno == journal.npages-1) { // create a page at end
    on_journalNewPageEnd_activate(menuitem, user_data);
    return;
  }
  do_switch_page(ui.pageno+1, TRUE, FALSE);
}


void
on_viewLastPage_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  do_switch_page(journal.npages-1, TRUE, FALSE);
}


void
on_viewShowLayer_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  if (ui.layerno == ui.cur_page->nlayers-1) return;
  reset_selection();
  ui.layerno++;
  ui.cur_layer = g_list_nth_data(ui.cur_page->layers, ui.layerno);
  gnome_canvas_item_show(GNOME_CANVAS_ITEM(ui.cur_layer->group));
  update_page_stuff();
}


void
on_viewHideLayer_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  if (ui.layerno == -1) return;
  reset_selection();
  gnome_canvas_item_hide(GNOME_CANVAS_ITEM(ui.cur_layer->group));
  ui.layerno--;
  if (ui.layerno<0) ui.cur_layer = NULL;
  else ui.cur_layer = g_list_nth_data(ui.cur_page->layers, ui.layerno);
  update_page_stuff();
}


void
on_journalNewPageBefore_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct Page *pg;

  end_text();
  reset_selection();
  pg = new_page(ui.cur_page);
  journal.pages = g_list_insert(journal.pages, pg, ui.pageno);
  journal.npages++;
  do_switch_page(ui.pageno, TRUE, TRUE);
  
  prepare_new_undo();
  undo->type = ITEM_NEW_PAGE;
  undo->val = ui.pageno;
  undo->page = pg;
}


void
on_journalNewPageAfter_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct Page *pg;

  end_text();
  reset_selection();
  pg = new_page(ui.cur_page);
  journal.pages = g_list_insert(journal.pages, pg, ui.pageno+1);
  journal.npages++;
  do_switch_page(ui.pageno+1, TRUE, TRUE);

  prepare_new_undo();
  undo->type = ITEM_NEW_PAGE;
  undo->val = ui.pageno;
  undo->page = pg;
}


void
on_journalNewPageEnd_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct Page *pg;

  end_text();
  reset_selection();
  pg = new_page((struct Page *)g_list_last(journal.pages)->data);
  journal.pages = g_list_append(journal.pages, pg);
  journal.npages++;
  do_switch_page(journal.npages-1, TRUE, TRUE);

  prepare_new_undo();
  undo->type = ITEM_NEW_PAGE;
  undo->val = ui.pageno;
  undo->page = pg;
}


void
on_journalDeletePage_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GList *layerlist, *itemlist;
  struct Layer *l;

  end_text();
  if (journal.npages == 1) return;
  reset_selection();  
  reset_recognizer(); // safer 
  prepare_new_undo();
  undo->type = ITEM_DELETE_PAGE;
  undo->val = ui.pageno;
  undo->page = ui.cur_page;

  // unmap all the canvas items  
  gtk_object_destroy(GTK_OBJECT(ui.cur_page->group));
  ui.cur_page->group = NULL;
  ui.cur_page->bg->canvas_item = NULL;
  for (layerlist = ui.cur_page->layers; layerlist!=NULL; layerlist = layerlist->next) {
    l = (struct Layer *)layerlist->data;
    for (itemlist = l->items; itemlist!=NULL; itemlist = itemlist->next)
      ((struct Item *)itemlist->data)->canvas_item = NULL;
    l->group = NULL;
  }
  
  journal.pages = g_list_remove(journal.pages, ui.cur_page);
  journal.npages--;
  if (ui.pageno == journal.npages) ui.pageno--;
  ui.cur_page = NULL;
     // so do_switch_page() won't try to remap the layers of the defunct page
  do_switch_page(ui.pageno, TRUE, TRUE);
}


void
on_journalNewLayer_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct Layer *l;
  
  end_text();
  reset_selection();
  l = g_new(struct Layer, 1);
  l->items = NULL;
  l->nitems = 0;
  l->group = (GnomeCanvasGroup *) gnome_canvas_item_new(
    ui.cur_page->group, gnome_canvas_group_get_type(), NULL);
  lower_canvas_item_to(ui.cur_page->group, GNOME_CANVAS_ITEM(l->group),
    (ui.cur_layer!=NULL)?(GNOME_CANVAS_ITEM(ui.cur_layer->group)):(ui.cur_page->bg->canvas_item));
  ui.cur_page->layers = g_list_insert(ui.cur_page->layers, l, ui.layerno+1);
  ui.cur_layer = l;
  ui.layerno++;
  ui.cur_page->nlayers++;
  update_page_stuff();

  prepare_new_undo();
  undo->type = ITEM_NEW_LAYER;
  undo->val = ui.layerno;
  undo->layer = l;
  undo->page = ui.cur_page;  
}


void
on_journalDeleteLayer_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GList *list;
  
  end_text();
  if (ui.cur_layer == NULL) return;
  reset_selection();
  reset_recognizer(); // safer
  prepare_new_undo();
  undo->type = ITEM_DELETE_LAYER;
  undo->val = ui.layerno;
  undo->layer = ui.cur_layer;
  undo->layer2 = NULL;
  undo->page = ui.cur_page;
  // delete all the canvas items
  gtk_object_destroy(GTK_OBJECT(ui.cur_layer->group));
  ui.cur_layer->group = NULL;
  for (list=ui.cur_layer->items; list!=NULL; list=list->next)
    ((struct Item *)list->data)->canvas_item = NULL;

  ui.cur_page->layers = g_list_remove(ui.cur_page->layers, ui.cur_layer);

  if (ui.cur_page->nlayers>=2) {
    ui.cur_page->nlayers--;
    ui.layerno--;
    if (ui.layerno<0) ui.cur_layer = NULL;
    else ui.cur_layer = (struct Layer *)g_list_nth_data(ui.cur_page->layers, ui.layerno);
  } 
  else { // special case: can't remove the last layer
    ui.cur_layer = g_new(struct Layer, 1);
    ui.cur_layer->items = NULL;
    ui.cur_layer->nitems = 0;
    ui.cur_layer->group = (GnomeCanvasGroup *) gnome_canvas_item_new(
      ui.cur_page->group, gnome_canvas_group_get_type(), NULL);
    ui.cur_page->layers = g_list_append(NULL, ui.cur_layer);
    undo->val = -1;
    undo->layer2 = ui.cur_layer;
  }

  update_page_stuff();
#ifdef USE_HILDON
  {
    gchar info_str[512];
    GtkWidget *banner;

    sprintf (info_str, _("Current layer deleted. Use Undo to undo action..."));
    banner = hildon_banner_show_information (GTK_WIDGET(winMain), NULL, info_str);
    hildon_banner_set_timeout (HILDON_BANNER(banner), 1000);

    gtk_widget_destroy (banner);
  }
#endif
}


void
on_journalFlatten_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


// the paper sizes dialog

GtkWidget *papersize_dialog;
int papersize_std, papersize_unit;
double papersize_width, papersize_height;
gboolean papersize_need_init, papersize_width_valid, papersize_height_valid;

#define STD_SIZE_A4 0
#define STD_SIZE_A4R 1
#define STD_SIZE_LETTER 2
#define STD_SIZE_LETTER_R 3
#define STD_SIZE_CUSTOM 4

double unit_sizes[4] = {28.346, 72., 72./DISPLAY_DPI_DEFAULT, 1.};
double std_widths[STD_SIZE_CUSTOM] =  {595.27, 841.89, 612., 792.};
double std_heights[STD_SIZE_CUSTOM] = {841.89, 595.27, 792., 612.};
double std_units[STD_SIZE_CUSTOM] = {UNIT_CM, UNIT_CM, UNIT_IN, UNIT_IN};

void
on_journalPaperSize_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int i, response;
  struct Page *pg;
  GList *pglist;
  
  end_text();
  papersize_dialog = create_papersizeDialog();
  papersize_width = ui.cur_page->width;
  papersize_height = ui.cur_page->height;
  papersize_unit = ui.default_unit;
  unit_sizes[UNIT_PX] = 1./DEFAULT_ZOOM;
//  if (ui.cur_page->bg->type == BG_PIXMAP) papersize_unit = UNIT_PX;
  papersize_std = STD_SIZE_CUSTOM;
  for (i=0;i<STD_SIZE_CUSTOM;i++)
    if (fabs(papersize_width - std_widths[i])<0.1 &&
        fabs(papersize_height - std_heights[i])<0.1)
      { papersize_std = i; papersize_unit = std_units[i]; }
  papersize_need_init = TRUE;
  papersize_width_valid = papersize_height_valid = TRUE;
      
  gtk_widget_show(papersize_dialog);
  on_comboStdSizes_changed(GTK_COMBO_BOX(g_object_get_data(
       G_OBJECT(papersize_dialog), "comboStdSizes")), NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(papersize_dialog), GTK_RESPONSE_OK);
       
  response = gtk_dialog_run(GTK_DIALOG(papersize_dialog));
  gtk_widget_destroy(papersize_dialog);
  if (response != GTK_RESPONSE_OK) return;

  pg = ui.cur_page;
  for (pglist = journal.pages; pglist!=NULL; pglist = pglist->next) {
    if (ui.bg_apply_all_pages) pg = (struct Page *)pglist->data;
    prepare_new_undo();
    if (ui.bg_apply_all_pages) {
      if (pglist->next!=NULL) undo->multiop |= MULTIOP_CONT_REDO;
      if (pglist->prev!=NULL) undo->multiop |= MULTIOP_CONT_UNDO;
    }
    undo->type = ITEM_PAPER_RESIZE;
    undo->page = pg;
    undo->val_x = pg->width;
    undo->val_y = pg->height;
    if (papersize_width_valid) pg->width = papersize_width;
    if (papersize_height_valid) pg->height = papersize_height;
    make_page_clipbox(pg);
    update_canvas_bg(pg);
    if (!ui.bg_apply_all_pages) break;
  }
  do_switch_page(ui.pageno, TRUE, TRUE);
}


void
on_papercolorWhite_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_papercolor_activate(menuitem, COLOR_WHITE, predef_bgcolors_rgba[COLOR_WHITE]);
}


void
on_papercolorYellow_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_papercolor_activate(menuitem, COLOR_YELLOW, predef_bgcolors_rgba[COLOR_YELLOW]);
}


void
on_papercolorPink_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_papercolor_activate(menuitem, COLOR_RED, predef_bgcolors_rgba[COLOR_RED]);
}


void
on_papercolorOrange_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_papercolor_activate(menuitem, COLOR_ORANGE, predef_bgcolors_rgba[COLOR_ORANGE]);
}


void
on_papercolorBlue_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_papercolor_activate(menuitem, COLOR_BLUE, predef_bgcolors_rgba[COLOR_BLUE]);
}


void
on_papercolorGreen_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_papercolor_activate(menuitem, COLOR_GREEN, predef_bgcolors_rgba[COLOR_GREEN]);
}


void
on_papercolorOther_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog;
  GtkColorSelection *colorsel;
  gint result;
  guint rgba;
  GdkColor gdkcolor;
  
  end_text();
  dialog = gtk_color_selection_dialog_new(_("Pick a Paper Color"));
  colorsel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel);
  if (ui.cur_page->bg->type == BG_SOLID) rgba = ui.cur_page->bg->color_rgba;
  else rgba = ui.default_page.bg->color_rgba;
  rgb_to_gdkcolor(rgba, &gdkcolor);
  gtk_color_selection_set_current_color(colorsel, &gdkcolor);
  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_color_selection_get_current_color(colorsel, &gdkcolor);
    process_papercolor_activate(menuitem, COLOR_OTHER, gdkcolor_to_rgba(gdkcolor, 0xffff));
  }
  gtk_widget_destroy(dialog);
}


void
on_paperstylePlain_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_paperstyle_activate(menuitem, RULING_NONE);
}


void
on_paperstyleLined_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_paperstyle_activate(menuitem, RULING_LINED);
}


void
on_paperstyleRuled_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_paperstyle_activate(menuitem, RULING_RULED);
}


void
on_paperstyleGraph_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  process_paperstyle_activate(menuitem, RULING_GRAPH);
}


void
on_journalLoadBackground_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog, *attach_opt;
  struct Background *bg;
  struct Page *pg;
  int pageno;
  GList *bglist, *bglistiter;
  GtkFileFilter *filt_all, *filt_pix, *filt_pspdf;
  char *filename;
  gboolean attach;
#ifdef USE_HILDON
  HildonFileSystemModel *model = NULL;
#endif
  
  end_text();

#ifdef USE_HILDON
  model = HILDON_FILE_SYSTEM_MODEL(g_object_new(HILDON_TYPE_FILE_SYSTEM_MODEL,
			  "ref_widget", GTK_WIDGET(winMain), NULL));

  dialog = hildon_file_chooser_dialog_new_with_properties (
		  GTK_WINDOW(winMain),
		  "file_system_model", model,
		  "action", GTK_FILE_CHOOSER_ACTION_OPEN,
		  NULL);
#else
  dialog = gtk_file_chooser_dialog_new(_("Open Background"), GTK_WINDOW (winMain),
     GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
#ifdef FILE_DIALOG_SIZE_BUGFIX
  gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
#endif
#endif

  filt_all = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_all, _("All files"));
  gtk_file_filter_add_pattern(filt_all, "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_all);

#if GTK_CHECK_VERSION(2,6,0)

  if (!gtk_check_version(2, 6, 0)) {
    filt_pix = gtk_file_filter_new();
    gtk_file_filter_set_name(filt_pix, _("Bitmap files"));
    gtk_file_filter_add_pixbuf_formats(filt_pix);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_pix);
  }
  
#endif

  filt_pspdf = gtk_file_filter_new();
  gtk_file_filter_set_name(filt_pspdf, _("PS/PDF files (as bitmaps)"));
  gtk_file_filter_add_pattern(filt_pspdf, "*.ps");
  gtk_file_filter_add_pattern(filt_pspdf, "*.PS");
  gtk_file_filter_add_pattern(filt_pspdf, "*.pdf");
  gtk_file_filter_add_pattern(filt_pspdf, "*.PDF");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filt_pspdf);

  attach_opt = gtk_check_button_new_with_label(_("Attach file to the journal"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attach_opt), FALSE);
#ifdef USE_HILDON
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG (dialog)->vbox), attach_opt,
		  FALSE, TRUE, 6);
#else
  gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER (dialog), attach_opt);
#endif

  if (ui.default_path!=NULL) gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), ui.default_path);
  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    return;
  }
  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
  attach = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attach_opt));
  gtk_widget_destroy(dialog);
  
  set_cursor_busy(TRUE);
  bg = attempt_load_pix_bg(filename, attach);
  if (bg != NULL) bglist = g_list_append(NULL, bg);
  else bglist = attempt_load_gv_bg(filename);
  set_cursor_busy(FALSE);
  
  if (bglist == NULL) {
#ifdef USE_HILDON
    gchar descr[strlen(_("Error opening background ''"))+ strlen(g_basename(filename))];
    sprintf (descr, _("Error opening background '%s'"), g_basename(filename));
    dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
#else
    dialog = gtk_message_dialog_new(GTK_WINDOW(winMain), GTK_DIALOG_MODAL,
      GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
      _("Error opening background '%s'"), filename);
#endif
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(filename);
    return;
  }

  g_free(filename);
  reset_selection();
  pageno = ui.pageno;

  for (bglistiter = bglist, pageno = ui.pageno; 
           bglistiter!=NULL; bglistiter = bglistiter->next, pageno++) {
    prepare_new_undo();
    if (bglistiter->next!=NULL) undo->multiop |= MULTIOP_CONT_REDO;
    if (bglistiter->prev!=NULL) undo->multiop |= MULTIOP_CONT_UNDO;

    bg = (struct Background *)bglistiter->data;
    
    if (pageno == journal.npages) {
      undo->type = ITEM_NEW_PAGE;
      pg = new_page_with_bg(bg, 
              gdk_pixbuf_get_width(bg->pixbuf)/bg->pixbuf_scale,
              gdk_pixbuf_get_height(bg->pixbuf)/bg->pixbuf_scale);
      journal.pages = g_list_append(journal.pages, pg);
      journal.npages++;
      undo->val = pageno;
      undo->page = pg;
    } else
    {
      pg = g_list_nth_data(journal.pages, pageno);
      undo->type = ITEM_NEW_BG_RESIZE;
      undo->page = pg;
      undo->bg = pg->bg;
      bg->canvas_item = undo->bg->canvas_item;
      undo->bg->canvas_item = NULL;
      undo->val_x = pg->width;
      undo->val_y = pg->height;
      pg->bg = bg;
      pg->width = gdk_pixbuf_get_width(bg->pixbuf)/bg->pixbuf_scale;
      pg->height = gdk_pixbuf_get_height(bg->pixbuf)/bg->pixbuf_scale;
      make_page_clipbox(pg);
      update_canvas_bg(pg);
    }
  }

  g_list_free(bglist);
  if (ui.zoom != DEFAULT_ZOOM) {
    ui.zoom = DEFAULT_ZOOM;
    gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
    rescale_text_items();
    rescale_bg_pixmaps();
  }
  do_switch_page(ui.pageno, TRUE, TRUE);
}

void
on_journalScreenshot_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct Background *bg;
  
  end_text();
  reset_selection();
  gtk_window_iconify(GTK_WINDOW(winMain)); // hide ourselves
  gdk_display_sync(gdk_display_get_default());

  if (ui.cursor!=NULL)
    gdk_cursor_unref(ui.cursor);
  ui.cursor = gdk_cursor_new(GDK_TCROSS);

  bg = attempt_screenshot_bg();
    
  gtk_window_deiconify(GTK_WINDOW(winMain));
  update_cursor();
  if (bg==NULL) return;

  prepare_new_undo();
  undo->type = ITEM_NEW_BG_RESIZE;
  undo->page = ui.cur_page;
  undo->bg = ui.cur_page->bg;
  bg->canvas_item = undo->bg->canvas_item;
  undo->bg->canvas_item = NULL;
  undo->val_x = ui.cur_page->width;
  undo->val_y = ui.cur_page->height;

  ui.cur_page->bg = bg;
  ui.cur_page->width = gdk_pixbuf_get_width(bg->pixbuf)/bg->pixbuf_scale;
  ui.cur_page->height = gdk_pixbuf_get_height(bg->pixbuf)/bg->pixbuf_scale;

  make_page_clipbox(ui.cur_page);
  update_canvas_bg(ui.cur_page);

  if (ui.zoom != DEFAULT_ZOOM) {
    ui.zoom = DEFAULT_ZOOM;
    gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
    rescale_text_items();
    rescale_bg_pixmaps();
  }
  do_switch_page(ui.pageno, TRUE, TRUE);
}


void
on_journalApplyAllPages_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gboolean active;
  
  end_text();
  active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  if (active == ui.bg_apply_all_pages) return;
  ui.bg_apply_all_pages = active;
  update_page_stuff();
  
/* THIS IS THE OLD VERSION OF THE FEATURE -- APPLIED CURRENT BG TO ALL
  struct Page *page;
  GList *pglist;
  
  if (ui.cur_page->bg->type != BG_SOLID) return;
  reset_selection();
  for (pglist = journal.pages; pglist!=NULL; pglist = pglist->next) {
    page = (struct Page *)pglist->data;
    prepare_new_undo();
    undo->type = ITEM_NEW_BG_RESIZE;
    undo->page = page;
    undo->bg = page->bg;
    undo->val_x = page->width;
    undo->val_y = page->height; 
    if (pglist->next!=NULL) undo->multiop |= MULTIOP_CONT_REDO;
    if (pglist->prev!=NULL) undo->multiop |= MULTIOP_CONT_UNDO;
    page->bg = (struct Background *)g_memdup(ui.cur_page->bg, sizeof(struct Background));
    page->width = ui.cur_page->width;
    page->height = ui.cur_page->height;
    page->bg->canvas_item = undo->bg->canvas_item;
    undo->bg->canvas_item = NULL;
  
    make_page_clipbox(page);
    update_canvas_bg(page);
  }
  do_switch_page(ui.pageno, TRUE, TRUE);
*/

}


void
on_toolsPen_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (menuitem) {
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_RADIO_MENU_ITEM) {
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem)))
      return;
  } else {
    if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem)))
      return;
  }
  }
  
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return; // not user-generated
  if (ui.toolno[ui.cur_mapping] == TOOL_PEN) return;

  ui.cur_mapping = 0; // don't use switch_mapping() (refreshes buttons too soon)
  end_text();
  reset_selection();
  ui.toolno[ui.cur_mapping] = TOOL_PEN;
  ui.cur_brush = &(ui.brushes[ui.cur_mapping][TOOL_PEN]);
  ui.cur_brush->ruler = ui.default_brushes[TOOL_PEN].ruler;
  ui.cur_brush->recognizer = ui.default_brushes[TOOL_PEN].recognizer;
  update_mapping_linkings(TOOL_PEN);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsEraser_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (menuitem) {
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_RADIO_MENU_ITEM) {
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem)))
      return;
  } else {
    if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem)))
      return;
  }
  }
  
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return; // not user-generated
  if (ui.toolno[ui.cur_mapping] == TOOL_ERASER) return;

  ui.cur_mapping = 0; // don't use switch_mapping() (refreshes buttons too soon)
  end_text();
  reset_selection();
  ui.toolno[ui.cur_mapping] = TOOL_ERASER;
  ui.cur_brush = &(ui.brushes[ui.cur_mapping][TOOL_ERASER]);
  update_mapping_linkings(TOOL_ERASER);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsHighlighter_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (menuitem) {
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_RADIO_MENU_ITEM) {
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem)))
      return;
  } else {
    if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem)))
      return;
  }
  }
  
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return; // not user-generated
  if (ui.toolno[ui.cur_mapping] == TOOL_HIGHLIGHTER) return;

  ui.cur_mapping = 0; // don't use switch_mapping() (refreshes buttons too soon)
  end_text();
  reset_selection();
  ui.toolno[ui.cur_mapping] = TOOL_HIGHLIGHTER;
  ui.cur_brush = &(ui.brushes[ui.cur_mapping][TOOL_HIGHLIGHTER]);
//#ifndef USE_HILDON
  ui.cur_brush->ruler = ui.default_brushes[TOOL_HIGHLIGHTER].ruler;
//#endif
  ui.cur_brush->recognizer = ui.default_brushes[TOOL_HIGHLIGHTER].recognizer;
  update_mapping_linkings(TOOL_HIGHLIGHTER);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsText_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (menuitem) {
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_RADIO_MENU_ITEM) {
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem)))
      return;
  } else {
    if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem)))
      return;
  }
  }

  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return; // not user-generated
  if (ui.toolno[ui.cur_mapping] == TOOL_TEXT) return;

  ui.cur_mapping = 0; // don't use switch_mapping() (refreshes buttons too soon)
  reset_selection();
  ui.toolno[ui.cur_mapping] = TOOL_TEXT;
  ui.cur_brush = &(ui.brushes[ui.cur_mapping][TOOL_PEN]);
  update_mapping_linkings(-1);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsSelectRegion_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_toolsSelectRectangle_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_RADIO_MENU_ITEM) {
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem)))
      return;
  } else {
    if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem)))
      return;
  }
  
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return; // not user-generated
  if (ui.toolno[ui.cur_mapping] == TOOL_SELECTRECT) return;

  ui.cur_mapping = 0; // don't use switch_mapping() (refreshes buttons too soon)
  end_text();
  ui.toolno[ui.cur_mapping] = TOOL_SELECTRECT;
  update_mapping_linkings(-1);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsVerticalSpace_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (menuitem) {
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_RADIO_MENU_ITEM) {
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem)))
      return;
  } else {
    if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem)))
      return;
  }
  }
  
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return; // not user-generated
  if (ui.toolno[ui.cur_mapping] == TOOL_VERTSPACE) return;

  ui.cur_mapping = 0; // don't use switch_mapping() (refreshes buttons too soon)
  end_text();
  reset_selection();
  ui.toolno[ui.cur_mapping] = TOOL_VERTSPACE;
  update_mapping_linkings(-1);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_colorBlack_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_BLACK, predef_colors_rgba[COLOR_BLACK]);
}


void
on_colorBlue_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_BLUE, predef_colors_rgba[COLOR_BLUE]);
}


void
on_colorRed_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_RED, predef_colors_rgba[COLOR_RED]);
}


void
on_colorGreen_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_GREEN, predef_colors_rgba[COLOR_GREEN]);
}


void
on_colorGray_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_GRAY, predef_colors_rgba[COLOR_GRAY]);
}


void
on_colorLightBlue_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_LIGHTBLUE, predef_colors_rgba[COLOR_LIGHTBLUE]);
}


void
on_colorLightGreen_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_LIGHTGREEN, predef_colors_rgba[COLOR_LIGHTGREEN]);
}


void
on_colorMagenta_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_MAGENTA, predef_colors_rgba[COLOR_MAGENTA]);
}


void
on_colorOrange_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_ORANGE, predef_colors_rgba[COLOR_ORANGE]);
}


void
on_colorYellow_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_YELLOW, predef_colors_rgba[COLOR_YELLOW]);
}


void
on_colorWhite_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_color_activate(menuitem, COLOR_WHITE, predef_colors_rgba[COLOR_WHITE]);
}


void
on_colorOther_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_button_clicked(GTK_BUTTON(GET_COMPONENT("buttonColorChooser")));
}


void
on_penthicknessVeryFine_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_PEN, THICKNESS_VERYFINE);
}


void
on_penthicknessFine_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_PEN, THICKNESS_FINE);
}


void
on_penthicknessMedium_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_PEN, THICKNESS_MEDIUM);
}


void
on_penthicknessThick_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_PEN, THICKNESS_THICK);
}


void
on_penthicknessVeryThick_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_PEN, THICKNESS_VERYTHICK);
}


void
on_eraserFine_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_ERASER, THICKNESS_FINE);
}


void
on_eraserMedium_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_ERASER, THICKNESS_MEDIUM);
}


void
on_eraserThick_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_ERASER, THICKNESS_THICK);
}


void
on_eraserStandard_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem))) return;
  end_text();
  ui.brushes[0][TOOL_ERASER].tool_options = TOOLOPT_ERASER_STANDARD;
  update_mapping_linkings(TOOL_ERASER);
}


void
on_eraserWhiteout_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem))) return;
  end_text();
  ui.brushes[0][TOOL_ERASER].tool_options = TOOLOPT_ERASER_WHITEOUT;
  update_mapping_linkings(TOOL_ERASER);
}


void
on_eraserDeleteStrokes_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem))) return;
  end_text();
  ui.brushes[0][TOOL_ERASER].tool_options = TOOLOPT_ERASER_STROKES;
  update_mapping_linkings(TOOL_ERASER);
}


void
on_highlighterFine_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_HIGHLIGHTER, THICKNESS_FINE);
}


void
on_highlighterMedium_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_HIGHLIGHTER, THICKNESS_MEDIUM);
}


void
on_highlighterThick_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_thickness_activate(menuitem, TOOL_HIGHLIGHTER, THICKNESS_THICK);
}


void
on_toolsTextFont_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog;
  gchar *str;
  
  dialog = gtk_font_selection_dialog_new(_("Select Font"));
  str = make_cur_font_name();
  gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), str);
  g_free(str);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    return;
  }
  str = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  process_font_sel(str);
}    

void
on_toolsDefaultPen_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  switch_mapping(0);
  end_text();
  reset_selection();
  g_memmove(&(ui.brushes[0][TOOL_PEN]), ui.default_brushes+TOOL_PEN, sizeof(struct Brush));
  ui.toolno[0] = TOOL_PEN;
  ui.cur_brush = &(ui.brushes[0][TOOL_PEN]);
  update_mapping_linkings(TOOL_PEN);
  update_tool_buttons();
  update_tool_menu();
  update_pen_props_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsDefaultEraser_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  switch_mapping(0);
  end_text();
  reset_selection();
  g_memmove(&(ui.brushes[0][TOOL_ERASER]), ui.default_brushes+TOOL_ERASER, sizeof(struct Brush));
  ui.toolno[0] = TOOL_ERASER;
  ui.cur_brush = &(ui.brushes[0][TOOL_ERASER]);
  update_mapping_linkings(TOOL_ERASER);
  update_tool_buttons();
  update_tool_menu();
  update_eraser_props_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsDefaultHighlighter_activate    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  switch_mapping(0);
  end_text();
  reset_selection();
  g_memmove(&(ui.brushes[0][TOOL_HIGHLIGHTER]), ui.default_brushes+TOOL_HIGHLIGHTER, sizeof(struct Brush));
  ui.toolno[0] = TOOL_HIGHLIGHTER;
  ui.cur_brush = &(ui.brushes[0][TOOL_HIGHLIGHTER]);
  update_mapping_linkings(TOOL_HIGHLIGHTER);
  update_tool_buttons();
  update_tool_menu();
  update_highlighter_props_menu();
  update_color_menu();
  update_cursor();
}

void
on_toolsDefaultText_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  switch_mapping(0);
  if (ui.toolno[0]!=TOOL_TEXT) end_text();
  reset_selection();
  ui.toolno[0] = TOOL_TEXT;
  ui.cur_brush = &(ui.brushes[0][TOOL_PEN]);
  ui.cur_brush->color_no = ui.default_brushes[TOOL_PEN].color_no;
  ui.cur_brush->color_rgba = ui.default_brushes[TOOL_PEN].color_rgba;
  g_free(ui.font_name);
  ui.font_name = g_strdup(ui.default_font_name);
  ui.font_size = ui.default_font_size;
  if (ui.cur_item_type == ITEM_TEXT) {
    refont_text_item(ui.cur_item, ui.font_name, ui.font_size);
  }
  update_font_button();
  update_mapping_linkings(-1);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_toolsSetAsDefault_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct Item *it;
  
  if (ui.cur_mapping!=0 && !ui.button_switch_mapping) return;
  if (ui.toolno[ui.cur_mapping] < NUM_STROKE_TOOLS)
    g_memmove(ui.default_brushes+ui.toolno[ui.cur_mapping], 
              &(ui.brushes[ui.cur_mapping][ui.toolno[ui.cur_mapping]]), sizeof(struct Brush));
  if (ui.toolno[ui.cur_mapping] == TOOL_TEXT) {
    if (ui.cur_item_type == ITEM_TEXT) {
      g_free(ui.font_name);
      ui.font_name = g_strdup(ui.cur_item->font_name);
      ui.font_size = ui.cur_item->font_size;
    }
    else if (ui.selection!=NULL && ui.selection->items!=NULL &&
             ui.selection->items->next==NULL &&
             (it=(struct Item*)ui.selection->items->data)->type == ITEM_TEXT) {
      g_free(ui.font_name);
      ui.font_name = g_strdup(it->font_name);
      ui.font_size = it->font_size;
    }
    g_free(ui.default_font_name);
    ui.default_font_name = g_strdup(ui.font_name);
    ui.default_font_size = ui.font_size;
  }
  end_text();
}


void
on_toolsRuler_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gboolean active, current;
  
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_CHECK_MENU_ITEM)
    active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  else
    active = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem));

#ifdef USE_HILDON
  active = !ui.cur_brush->ruler;
#endif

  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return;
  current = (ui.toolno[ui.cur_mapping] == TOOL_PEN || ui.toolno[ui.cur_mapping] == TOOL_HIGHLIGHTER) && ui.cur_brush->ruler;
  if (active == current) return;
  
  ui.cur_mapping = 0;  
  end_text();
  if (ui.toolno[ui.cur_mapping]!=TOOL_PEN && ui.toolno[ui.cur_mapping]!=TOOL_HIGHLIGHTER) {
    reset_selection();
    ui.toolno[ui.cur_mapping] = TOOL_PEN;
    ui.cur_brush = &(ui.brushes[ui.cur_mapping][TOOL_PEN]);
    update_color_menu();
    update_tool_buttons();
    update_tool_menu();
    update_cursor();
  }
  
  ui.cur_brush->ruler = active;

#ifndef USE_HILDON
  if (active) ui.cur_brush->recognizer = FALSE;
#endif

  update_mapping_linkings(ui.toolno[ui.cur_mapping]);
  update_ruler_indicator();
}


void
on_toolsReco_activate                  (GtkMenuItem *menuitem,
                                        gpointer         user_data)
{
  gboolean active, current;
  
  if (menuitem) {
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_CHECK_MENU_ITEM)
    active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  else
    active = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem));
  }

  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return;
  current = (ui.toolno[ui.cur_mapping] == TOOL_PEN || ui.toolno[ui.cur_mapping] == TOOL_HIGHLIGHTER) && ui.cur_brush->recognizer;
  if (active == current) return;
  
  ui.cur_mapping = 0;
  end_text();
  if (ui.toolno[ui.cur_mapping]!=TOOL_PEN && ui.toolno[ui.cur_mapping]!=TOOL_HIGHLIGHTER) {
    reset_selection();
    ui.toolno[ui.cur_mapping] = TOOL_PEN;
    ui.cur_brush = &(ui.brushes[ui.cur_mapping][TOOL_PEN]);
    update_color_menu();
    update_tool_buttons();
    update_tool_menu();
    update_cursor();
  }
  
  ui.cur_brush->recognizer = active;
  if (active) {
    ui.cur_brush->ruler = FALSE;
    reset_recognizer();
  }
  update_mapping_linkings(ui.toolno[ui.cur_mapping]);
  update_ruler_indicator();
}


void
on_optionsSavePreferences_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  save_config_to_file();
}


void
on_helpIndex_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_helpAbout_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *aboutDialog;
  GtkLabel *labelTitle;
  
  end_text();
  aboutDialog = create_aboutDialog ();
  labelTitle = GTK_LABEL(g_object_get_data(G_OBJECT(aboutDialog), "labelTitle"));
  gtk_label_set_markup(labelTitle, 
    "<span size=\"xx-large\" weight=\"bold\">Xournal " VERSION "</span>");
  gtk_dialog_run (GTK_DIALOG(aboutDialog));
  gtk_widget_destroy(aboutDialog);
}


void
on_buttonToolDefault_clicked           (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  if (ui.toolno[0]==TOOL_TEXT) {
    on_toolsDefaultText_activate(NULL, NULL);
    return;
  }
  end_text();
  switch_mapping(0);
  if (ui.toolno[0] < NUM_STROKE_TOOLS) {
    g_memmove(&(ui.brushes[0][ui.toolno[0]]), ui.default_brushes+ui.toolno[0], sizeof(struct Brush));
    update_mapping_linkings(ui.toolno[0]);
    update_thickness_buttons();
    update_color_buttons();
    update_color_menu();
    if (ui.toolno[0] == TOOL_PEN) update_pen_props_menu();
    if (ui.toolno[0] == TOOL_ERASER) update_eraser_props_menu();
    if (ui.toolno[0] == TOOL_HIGHLIGHTER) update_highlighter_props_menu();
    update_cursor();
  }
}


void
on_buttonFine_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return;
  process_thickness_activate((GtkMenuItem*)toolbutton, ui.toolno[ui.cur_mapping], THICKNESS_FINE);
}


void
on_buttonMedium_clicked                (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return;
  process_thickness_activate((GtkMenuItem*)toolbutton, ui.toolno[ui.cur_mapping], THICKNESS_MEDIUM);
}


void
on_buttonThick_clicked                 (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return;
  process_thickness_activate((GtkMenuItem*)toolbutton, ui.toolno[ui.cur_mapping], THICKNESS_THICK);
}


gboolean
on_canvas_button_press_event           (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  double pt[2];
  gboolean page_change;
  struct Page *tmppage;
  GtkWidget *dialog;
  int mapping;
  gboolean is_core;
  struct Item *item;
  GdkEvent scroll_event;

#ifdef INPUT_DEBUG
  printf("DEBUG: ButtonPress (%s) (x,y)=(%.2f,%.2f), button %d, modifier %x\n", 
    event->device->name, event->x, event->y, event->button, event->state);
#endif

  // abort any page changes pending in the spin button, and take the focus
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(GET_COMPONENT("spinPageNo")), ui.pageno+1);
  reset_focus();

  is_core = (event->device == gdk_device_get_core_pointer());
  if (!ui.use_xinput && !is_core) return FALSE;
  if (ui.use_xinput && is_core && ui.discard_corepointer) return FALSE;
  if (event->type != GDK_BUTTON_PRESS) return FALSE; 
    // double-clicks may have broken axes member (free'd) due to a bug in GDK

  if (event->button > 3) { // scroll wheel events! don't paint...
    if (ui.use_xinput && !gtk_check_version(2, 17, 0) && event->button <= 7) {
      /* with GTK+ 2.17 and later, the entire widget hierarchy is xinput-aware,
         so the core button event gets discarded and the scroll event never 
         gets processed by the main window. This is arguably a GTK+ bug.
         We work around it. */
      scroll_event.scroll.type = GDK_SCROLL;
      scroll_event.scroll.window = event->window;
      scroll_event.scroll.send_event = event->send_event;
      scroll_event.scroll.time = event->time;
      scroll_event.scroll.x = event->x;
      scroll_event.scroll.y = event->y;
      scroll_event.scroll.state = event->state;
      scroll_event.scroll.device = event->device;
      scroll_event.scroll.x_root = event->x_root;
      scroll_event.scroll.y_root = event->y_root;
      if (event->button == 4) scroll_event.scroll.direction = GDK_SCROLL_UP;
      else if (event->button == 5) scroll_event.scroll.direction = GDK_SCROLL_DOWN;
      else if (event->button == 6) scroll_event.scroll.direction = GDK_SCROLL_LEFT;
      else scroll_event.scroll.direction = GDK_SCROLL_RIGHT;
      gtk_widget_event(GET_COMPONENT("scrolledwindowMain"), &scroll_event);
    }
    return FALSE;
  }
  if ((event->state & (GDK_CONTROL_MASK|GDK_MOD1_MASK)) != 0) return FALSE;
    // no control-clicking or alt-clicking
  if (!is_core)
    fix_xinput_coords((GdkEvent *)event);

  if (!finite(event->x) || !finite(event->y)) return FALSE; // Xorg 7.3 bug

  if (ui.cur_item_type == ITEM_TEXT) {
    if (!is_event_within_textview(event)) end_text();
    else return FALSE;
  }
  if (ui.cur_item_type == ITEM_STROKE && ui.is_corestroke && !is_core &&
      ui.cur_path.num_points == 1) { 
      // Xorg 7.3+ sent core event before XInput event: fix initial point 
    ui.is_corestroke = FALSE;
    get_pointer_coords((GdkEvent *)event, ui.cur_path.coords);
  }
  if (ui.cur_item_type != ITEM_NONE) return FALSE; // we're already doing something

  // if button_switch_mapping enabled, button 2 or 3 clicks only switch mapping
  if (ui.button_switch_mapping && event->button > 1) {
    ui.which_unswitch_button = event->button;
    switch_mapping(event->button-1);
    return FALSE;
  }

  ui.is_corestroke = is_core;
  ui.stroke_device = event->device;

  if (ui.use_erasertip && event->device->source == GDK_SOURCE_ERASER)
    mapping = NUM_BUTTONS;
  else if (ui.button_switch_mapping) {
    mapping = ui.cur_mapping;
    if (!mapping && (event->state & GDK_BUTTON2_MASK)) mapping = 1;
    if (!mapping && (event->state & GDK_BUTTON3_MASK)) mapping = 2;
  }
  else mapping = event->button-1;

  // check whether we're in a page
  page_change = FALSE;
  tmppage = ui.cur_page;
  get_pointer_coords((GdkEvent *)event, pt);
  while (ui.view_continuous && (pt[1] < - VIEW_CONTINUOUS_SKIP)) {
    if (ui.pageno == 0) break;
    page_change = TRUE;
    ui.pageno--;
    tmppage = g_list_nth_data(journal.pages, ui.pageno);
    pt[1] += tmppage->height + VIEW_CONTINUOUS_SKIP;
  }
  while (ui.view_continuous && (pt[1] > tmppage->height + VIEW_CONTINUOUS_SKIP)) {
    if (ui.pageno == journal.npages-1) break;
    pt[1] -= tmppage->height + VIEW_CONTINUOUS_SKIP;
    page_change = TRUE;
    ui.pageno++;
    tmppage = g_list_nth_data(journal.pages, ui.pageno);
  }
  if (page_change) do_switch_page(ui.pageno, FALSE, FALSE);
  
  // can't paint on the background...

  if (ui.cur_layer == NULL) {
    /* warn */
#ifdef USE_HILDON
    gchar descr[strlen(_("Drawing is not allowed on the background layer"))];

    sprintf (descr, _("Drawing is not allowed on the background layer"));
    dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
#else
    dialog = gtk_message_dialog_new(GTK_WINDOW(winMain), GTK_DIALOG_MODAL,
      GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("Drawing is not allowed on the "
      "background layer.\n Switching to Layer 1."));
#endif
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    on_viewShowLayer_activate(NULL, NULL);
    return FALSE;
  }

  // switch mappings if needed
  
  ui.which_mouse_button = event->button;
  switch_mapping(mapping);

  // in text tool, clicking in a text area edits it
  if (ui.toolno[mapping] == TOOL_TEXT) {
    item = click_is_in_text(ui.cur_layer, pt[0], pt[1]);
    if (item!=NULL) { 
      reset_selection();
      start_text((GdkEvent *)event, item);
      return FALSE;
    }
  }

  // if this can be a selection move or resize, then it takes precedence over anything else  
  if (start_resizesel((GdkEvent *)event)) return FALSE;
  if (start_movesel((GdkEvent *)event)) return FALSE;
  
  if (ui.toolno[mapping] != TOOL_SELECTREGION && ui.toolno[mapping] != TOOL_SELECTRECT)
    reset_selection();

  // process the event
  
  if (ui.toolno[mapping] == TOOL_HAND) {
    ui.cur_item_type = ITEM_HAND;
    get_pointer_coords((GdkEvent *)event, ui.hand_refpt);
    ui.hand_refpt[0] += ui.cur_page->hoffset;
    ui.hand_refpt[1] += ui.cur_page->voffset;
  } 
  else if (ui.toolno[mapping] == TOOL_PEN || ui.toolno[mapping] == TOOL_HIGHLIGHTER ||
        (ui.toolno[mapping] == TOOL_ERASER && ui.cur_brush->tool_options == TOOLOPT_ERASER_WHITEOUT)) {
    create_new_stroke((GdkEvent *)event);
  } 
  else if (ui.toolno[mapping] == TOOL_ERASER) {
    ui.cur_item_type = ITEM_ERASURE;
    do_eraser((GdkEvent *)event, ui.cur_brush->thickness/2,
               ui.cur_brush->tool_options == TOOLOPT_ERASER_STROKES);
  }
  else if (ui.toolno[mapping] == TOOL_SELECTRECT) {
    start_selectrect((GdkEvent *)event);
  }
  else if (ui.toolno[mapping] == TOOL_VERTSPACE) {
    start_vertspace((GdkEvent *)event);
  }
  else if (ui.toolno[mapping] == TOOL_TEXT) {
    start_text((GdkEvent *)event, NULL);
  }
  return FALSE;
}


gboolean
on_canvas_button_release_event         (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  gboolean is_core;
  
#ifdef INPUT_DEBUG
  printf("DEBUG: ButtonRelease (%s) (x,y)=(%.2f,%.2f), button %d, modifier %x\n", 
      event->device->name, event->x, event->y, event->button, event->state);
#endif

  is_core = (event->device == gdk_device_get_core_pointer());
  if (!ui.use_xinput && !is_core) return FALSE;
  if (ui.use_xinput && is_core && !ui.is_corestroke) return FALSE;
  if (!is_core) fix_xinput_coords((GdkEvent *)event);

  if (event->button != ui.which_mouse_button && 
      event->button != ui.which_unswitch_button)
    return FALSE;

  if (ui.cur_item_type == ITEM_STROKE) {
    finalize_stroke();
    if (ui.cur_brush->recognizer) recognize_patterns();
  }
  else if (ui.cur_item_type == ITEM_ERASURE) {
    finalize_erasure();
  }
  else if (ui.cur_item_type == ITEM_SELECTRECT) {
    finalize_selectrect();
  }
  else if (ui.cur_item_type == ITEM_MOVESEL || ui.cur_item_type == ITEM_MOVESEL_VERT) {
    finalize_movesel();
  }
  else if (ui.cur_item_type == ITEM_RESIZESEL) {
    finalize_resizesel();
  }
  else if (ui.cur_item_type == ITEM_HAND) {
    ui.cur_item_type = ITEM_NONE;
  }

  if (!ui.which_unswitch_button || event->button == ui.which_unswitch_button)
    switch_mapping(0); // will reset ui.which_unswitch_button

  return FALSE;
}


gboolean
on_canvas_enter_notify_event           (GtkWidget       *widget,
                                        GdkEventCrossing *event,
                                        gpointer         user_data)
{
  GList *dev_list;
  GdkDevice *dev;

#ifdef INPUT_DEBUG
  printf("DEBUG: enter notify\n");
#endif
    /* re-enable input devices after they've been emergency-disabled
       by leave_notify */
  if (!gtk_check_version(2, 17, 0)) {
    for (dev_list = gdk_devices_list(); dev_list != NULL; dev_list = dev_list->next) {
      dev = GDK_DEVICE(dev_list->data);
      gdk_device_set_mode(dev, GDK_MODE_SCREEN);
    }
  }
  return FALSE;
}

gboolean
on_canvas_leave_notify_event           (GtkWidget       *widget,
                                        GdkEventCrossing *event,
                                        gpointer         user_data)
{
  GList *dev_list;
  GdkDevice *dev;

#ifdef INPUT_DEBUG
  printf("DEBUG: leave notify (mode=%d, details=%d)\n", event->mode, event->detail);
#endif
    /* emergency disable XInput to avoid segfaults (GTK+ 2.17) or 
       interface non-responsiveness (GTK+ 2.18) */
  if (!gtk_check_version(2, 17, 0)) { 
    for (dev_list = gdk_devices_list(); dev_list != NULL; dev_list = dev_list->next) {
      dev = GDK_DEVICE(dev_list->data);
      gdk_device_set_mode(dev, GDK_MODE_DISABLED);
    }
  }
							
  return FALSE;
}


gboolean
on_canvas_expose_event                 (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  if (ui.view_continuous && ui.progressive_bg) rescale_bg_pixmaps();
  return FALSE;
}

void
switch_fullscreen (gpointer user_data) {
	// let's go toggle fullscreen mode
	end_text ();
 
 	if (ui.fullscreen) {
	 	gtk_window_unfullscreen (GTK_WINDOW(user_data));
	 	ui.fullscreen = FALSE;
 	} else {
	 	gtk_window_fullscreen (GTK_WINDOW(user_data));
 		ui.fullscreen = TRUE;
 	}
		
	gtk_check_menu_item_set_active (
 		GTK_CHECK_MENU_ITEM(
			GET_COMPONENT("Fullscreen")),
 			ui.fullscreen);
}
			

gboolean
on_canvas_key_press_event              (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
  GtkAdjustment *adj;
  gint pgheight;

#ifdef USE_HILDON
   int cx, cy;
   int global_hildon_scroll_down = 10,
       global_hildon_scroll_down_ctrl = 100,
       global_hildon_scroll_up = 10,
       global_hildon_scroll_up_ctrl = 100,
       global_hildon_scroll_left = 10,
       global_hildon_scroll_left_ctrl = 100,
       global_hildon_scroll_right = 10,
       global_hildon_scroll_right_ctrl = 100;

   switch (event->keyval) {
	  case GDK_s:
		  if (event->state & GDK_CONTROL_MASK) {
 		  	on_fileSave_activate (NULL, user_data);
		  }
 		  return TRUE;
	  case GDK_n:
		  if (event->state & GDK_CONTROL_MASK) {
 		  	on_fileNew_activate (NULL, user_data);
		  }
 		  return TRUE;
	  case GDK_o:
		  if (event->state & GDK_CONTROL_MASK) {
 		 	on_fileOpen_activate (NULL, user_data);
		  } else {
 		 	 // let's zoom out
 		 	 if (ui.zoom < MIN_ZOOM) {
 				  return FALSE;
 			  }
 	
 			  ui.zoom /= ui.zoom_step_factor;
 			  gnome_canvas_set_pixels_per_unit (canvas, ui.zoom);
 			  rescale_text_items ();
 			  rescale_bg_pixmaps ();
		  }
 		  return TRUE;
	  case GDK_u:
		  on_editUndo_activate (NULL, user_data);

		  return TRUE;
	  case GDK_z:
		  if (event->state & GDK_CONTROL_MASK) {
 		  	on_editUndo_activate (NULL, user_data);
		  }
 		  return TRUE;
	  case GDK_r:
		  on_editRedo_activate (NULL, user_data);

		  return TRUE;
	  case GDK_y:
		  if (event->state & GDK_CONTROL_MASK) {
			on_editRedo_activate (NULL, user_data);
		  }
		  return TRUE;
	  case GDK_f:
		  switch_fullscreen (user_data);

		  return TRUE;
 	  case GDK_F6:
		  switch_fullscreen (user_data);
 		  return TRUE;
#ifdef HILDON_HARDKEY_INCREASE
	  case HILDON_HARDKEY_INCREASE:
#else
 	  case GDK_F7:
#endif
	  case GDK_i:
 		  // let's zoom in
 		  if (ui.zoom > MAX_ZOOM) {
 			  return FALSE;
 		  }
 
 		  ui.zoom *= ui.zoom_step_factor;
 		  gnome_canvas_set_pixels_per_unit (canvas, ui.zoom);
 		  rescale_text_items ();
 		  rescale_bg_pixmaps ();
 
 		  return TRUE;
#ifdef HILDON_HARDKEY_DECREASE
	  case HILDON_HARDKEY_DECREASE:
#else
 	  case GDK_F8:
#endif
 		  // let's zoom out
 		  if (ui.zoom < MIN_ZOOM) {
 			  return FALSE;
 		  }
 
 		  ui.zoom /= ui.zoom_step_factor;
 		  gnome_canvas_set_pixels_per_unit (canvas, ui.zoom);
 		  rescale_text_items ();
 		  rescale_bg_pixmaps ();
 
 		  return TRUE;
	  case HILDON_HARDKEY_UP:
		  gnome_canvas_get_scroll_offsets (canvas, &cx, &cy);
		  if (event->state & GDK_CONTROL_MASK) {
		  	cy-=global_hildon_scroll_down_ctrl;
		  } else {
			cy-=global_hildon_scroll_down;
		  }
		  gnome_canvas_scroll_to (canvas, cx, cy);

		  return TRUE;
	  case HILDON_HARDKEY_DOWN:
		  gnome_canvas_get_scroll_offsets (canvas, &cx, &cy);
		  if (event->state & GDK_CONTROL_MASK) {
		  	cy+=global_hildon_scroll_up_ctrl;
		  } else {
			cy+=global_hildon_scroll_up;
		  }
		  gnome_canvas_scroll_to (canvas, cx, cy);

		  return TRUE;
	  case HILDON_HARDKEY_LEFT:
		  gnome_canvas_get_scroll_offsets (canvas, &cx, &cy);
		  if (event->state & GDK_CONTROL_MASK) {
		  	cx-=global_hildon_scroll_left_ctrl;
		  } else {
			cx-=global_hildon_scroll_left;
		  }
		  gnome_canvas_scroll_to (canvas, cx, cy);

		  return TRUE;
	  case HILDON_HARDKEY_RIGHT:
		  gnome_canvas_get_scroll_offsets (canvas, &cx, &cy);
		  if (event->state & GDK_CONTROL_MASK) {
		  	cx+=global_hildon_scroll_right_ctrl;
		  } else {
			cx+=global_hildon_scroll_right;
		  }
		  gnome_canvas_scroll_to (canvas, cx, cy);

		  return TRUE;
   }
#endif
  // Esc leaves text edition, or leaves fullscreen mode
  if (event->keyval == GDK_Escape) {
    if (ui.cur_item_type == ITEM_TEXT) { 
      end_text(); 
      return TRUE;
    }
    else if (ui.fullscreen) {
      do_fullscreen(FALSE);
      return TRUE;
    }
    else return FALSE;
  }
 
  /* In single page mode, switch pages with PgUp/PgDn (or Up/Dn) 
     when there's nowhere else to go. */
  pgheight = GTK_WIDGET(canvas)->allocation.height;
  adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(GET_COMPONENT("scrolledwindowMain")));

  if (event->keyval == GDK_Page_Down || event->keyval == GDK_Down) {
    if (!ui.view_continuous && 
         (0.96 * ui.zoom * ui.cur_page->height < pgheight ||
          adj->value == adj->upper-pgheight)) 
    {
      end_text();
      if (ui.pageno < journal.npages-1)
        do_switch_page(ui.pageno+1, TRUE, FALSE);
      return TRUE;
    }
    if (adj->value == adj->upper-pgheight) return TRUE; // don't send focus away
  }

  if (event->keyval == GDK_Page_Up || event->keyval == GDK_Up) {
    if (!ui.view_continuous && 
         (0.96 * ui.zoom * ui.cur_page->height < pgheight ||
          adj->value == adj->lower))
    {
      end_text();
      if (ui.pageno != 0) {
        do_switch_page(ui.pageno-1, TRUE, FALSE);
        gtk_adjustment_set_value(adj, adj->upper-pgheight);
      }
      return TRUE;
    }
    if (adj->value == adj->lower) return TRUE; // don't send focus away
  }

  return FALSE;
}


gboolean
on_canvas_motion_notify_event          (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{
  gboolean looks_wrong, is_core;
  double pt[2];
  GdkModifierType mask;
#ifdef USE_HILDON
  gdouble pressure = 0.00;
#endif

  /* we don't care about this event unless some operation is in progress;
     or if there's a selection (then we might want to change the mouse
     cursor to indicate the possibility of resizing) */  
  if (ui.cur_item_type == ITEM_NONE && ui.selection==NULL) return FALSE;
  if (ui.cur_item_type == ITEM_TEXT) return FALSE;

  is_core = (event->device == gdk_device_get_core_pointer());
  if (!ui.use_xinput && !is_core) return FALSE;
  if (!is_core) fix_xinput_coords((GdkEvent *)event);
  if (!finite(event->x) || !finite(event->y)) return FALSE; // Xorg 7.3 bug

  if (ui.selection!=NULL && ui.cur_item_type == ITEM_NONE) {
    get_pointer_coords((GdkEvent *)event, pt);
    update_cursor_for_resize(pt);
    return FALSE;
  }

  if (ui.use_xinput && is_core && !ui.is_corestroke) return FALSE;
  if (!is_core) ui.is_corestroke = FALSE;

#ifdef INPUT_DEBUG
  printf("DEBUG: MotionNotify (%s) (x,y)=(%.2f,%.2f), modifier %x\n", 
    is_core?"core":"xinput", event->x, event->y, event->state);
#endif
  
  looks_wrong = !(event->state & (1<<(7+ui.which_mouse_button)));
  if (looks_wrong) {
    gdk_device_get_state(ui.stroke_device, event->window, NULL, &mask);
    looks_wrong = !(mask & (1<<(7+ui.which_mouse_button)));
  }
  
  if (looks_wrong) { /* mouse button shouldn't be up... give up */
    if (ui.cur_item_type == ITEM_STROKE) {
      finalize_stroke();
      if (ui.cur_brush->recognizer) recognize_patterns();
    }
    else if (ui.cur_item_type == ITEM_ERASURE) {
      finalize_erasure();
    }
    else if (ui.cur_item_type == ITEM_SELECTRECT) {
      finalize_selectrect();
    }
    else if (ui.cur_item_type == ITEM_MOVESEL || ui.cur_item_type == ITEM_MOVESEL_VERT) {
      finalize_movesel();
    }
    else if (ui.cur_item_type == ITEM_RESIZESEL) {
      finalize_resizesel();
    }
    switch_mapping(0);
    return FALSE;
  }
  
#if 0
#ifdef USE_HILDON
    if (gdk_event_get_axis ((GdkEvent*) event, GDK_AXIS_PRESSURE, &pressure) &&
		    (1-10*pressure) > 0.4) {
	    if (ui.cur_item_type != ITEM_HAND) {
		on_toolsHand_activate (NULL, NULL);
	    }
    }
#endif
#endif

  if (ui.cur_item_type == ITEM_STROKE) {
    continue_stroke((GdkEvent *)event);
  }
  else if (ui.cur_item_type == ITEM_ERASURE) {
    do_eraser((GdkEvent *)event, ui.cur_brush->thickness/2,
               ui.cur_brush->tool_options == TOOLOPT_ERASER_STROKES);
  }
  else if (ui.cur_item_type == ITEM_SELECTRECT) {
    get_pointer_coords((GdkEvent *)event, pt);
    ui.selection->bbox.right = pt[0];
    ui.selection->bbox.bottom = pt[1];
    gnome_canvas_item_set(ui.selection->canvas_item,
                               "x2", pt[0], "y2", pt[1], NULL);
  }
  else if (ui.cur_item_type == ITEM_MOVESEL || ui.cur_item_type == ITEM_MOVESEL_VERT) {
    continue_movesel((GdkEvent *)event);
  }
  else if (ui.cur_item_type == ITEM_RESIZESEL) {
    continue_resizesel((GdkEvent *)event);
  }
  else if (ui.cur_item_type == ITEM_HAND) {
    do_hand((GdkEvent *)event);
  }
  
  return FALSE;
}

void
on_comboLayer_changed                  (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
  int val;

  if (ui.in_update_page_stuff) return; // avoid a bad retroaction

  end_text();

#ifdef USE_HILDON
  val = hildon_touch_selector_get_active (HILDON_TOUCH_SELECTOR(combobox), 0);
  if (val <= -1) return;
  g_print ("raw val %d\n", val);

  if (val == (ui.layerbox_length-1)) {
    gchar info_str [512];
    GtkWidget *banner;

    on_journalNewLayer_activate (NULL,NULL);

    sprintf (info_str, _("Switched to newly added layer..."));
    banner = hildon_banner_show_information (GTK_WIDGET(winMain), NULL, info_str);
    hildon_banner_set_timeout (HILDON_BANNER(banner), 1000);

    gtk_widget_destroy (banner);
    return ;
  }

  if (val == ui.layerbox_length) {
    val = ui.cur_page->nlayers-1;
  }

  val = ui.cur_page->nlayers-val-1;
#else
  val = gtk_combo_box_get_active(combobox);
  if (val <= -1) return;
#endif
  g_print ("val = %d\nui.layerno = %d\nui.cur_page->nlayers %d\n", val, ui.layerno, ui.cur_page->nlayers);

  if (val == ui.layerno) return;

  reset_selection();
  while (val>ui.layerno) {
    ui.layerno++;
    ui.cur_layer = g_list_nth_data(ui.cur_page->layers, ui.layerno);
    gnome_canvas_item_show(GNOME_CANVAS_ITEM(ui.cur_layer->group));
  }
  while (val<ui.layerno) {
    gnome_canvas_item_hide(GNOME_CANVAS_ITEM(ui.cur_layer->group));
    ui.layerno--;
    if (ui.layerno<0) ui.cur_layer = NULL;
    else ui.cur_layer = g_list_nth_data(ui.cur_page->layers, ui.layerno);
  }
  update_page_stuff();
}


gboolean
on_winMain_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  end_text();
  
#ifdef USE_HILDON
  hildon_handle_saveunsaved (user_data);

//  if (!g_file_test(XOURNAL_SCREENSHOT_PATH, G_FILE_TEST_EXISTS)) {
//    hildon_gtk_window_take_screenshot (GTK_WINDOW(winMain), TRUE);
//  }
#else
  if (ok_to_close())
#endif
     gtk_main_quit();

  return TRUE;
}


void
on_optionsUseXInput_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  ui.allow_xinput = ui.use_xinput =
    gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));

/* HOW THINGS USED TO BE:

   We'd like on_canvas_... to get BOTH core and xinput events. Up to
   GTK+ 2.16 this is achieved by making only the canvas's parent 
   GdkWindow xinput-aware, rather than the entire hierarchy.
   Otherwise, the proximity detection code in GDK is broken and 
   we'll lose core events.
   
   Up to GTK+ 2.10, gtk_widget_set_extension_events() only sets
   extension events for the widget's main window itself; in GTK+ 2.11
   also traverses GDK child windows that belong to the widget
   and sets their extension events too. We want to avoid that.
   So we use gdk_input_set_extension_events() directly on the canvas.

   As much as possible, we'd like to keep doing this, though GTK+ 2.17
   is making our life harder (crasher bugs require us to disable XInput
   while editing text or using the layers combo box, but disabling
   XInput while in a XInput-aware window causes the interface to become
   non-responsive). 
*/

  if (!gtk_check_version(2, 17, 0)) {
    /* GTK+ 2.17 and later: everybody shares a single native window,
       so we'll never get any core events, and we might as well set 
       extension events the way we're supposed to. Doing so helps solve 
       crasher bugs in 2.17, and prevents us from losing two-button
       events in 2.18 */
    gtk_widget_set_extension_events(GTK_WIDGET (canvas), 
       ui.use_xinput?GDK_EXTENSION_EVENTS_ALL:GDK_EXTENSION_EVENTS_NONE);
  } else {
    /* GTK+ 2.16 and earlier: we only activate extension events on the
       canvas's parent GdkWindow. This allows us to keep receiving core
       events. */
    gdk_input_set_extension_events(GTK_WIDGET(canvas)->window, 
      GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
      ui.use_xinput?GDK_EXTENSION_EVENTS_ALL:GDK_EXTENSION_EVENTS_NONE);
  }

  update_mappings_menu();
}

void
on_vscroll_changed                     (GtkAdjustment   *adjustment,
                                        gpointer         user_data)
{
  gboolean need_update;
  double viewport_top, viewport_bottom;
  struct Page *tmppage;
  
  if (!ui.view_continuous) return;
  
  if (ui.progressive_bg) rescale_bg_pixmaps();
  need_update = FALSE;
  viewport_top = adjustment->value / ui.zoom;
  viewport_bottom = (adjustment->value + adjustment->page_size) / ui.zoom;
  tmppage = ui.cur_page;
  while (viewport_top > tmppage->voffset + tmppage->height) {
    if (ui.pageno == journal.npages-1) break;
    need_update = TRUE;
    ui.pageno++;
    tmppage = g_list_nth_data(journal.pages, ui.pageno);
  }
  while (viewport_bottom < tmppage->voffset) {
    if (ui.pageno == 0) break;
    need_update = TRUE;
    ui.pageno--;
    tmppage = g_list_nth_data(journal.pages, ui.pageno);
  }
  if (need_update) {
    end_text();
    do_switch_page(ui.pageno, FALSE, FALSE);
  }
  return;
}

void
on_spinPageNo_value_changed            (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  int val;

  if (ui.in_update_page_stuff) return; // avoid a bad retroaction

  /* in preparation for end_text(), send focus to the canvas if it's not ours.
     (avoid issues with Gtk trying to send focus to the dead text widget) */

  if (!GTK_WIDGET_HAS_FOCUS(spinbutton))
    gtk_widget_grab_focus(GTK_WIDGET(canvas));
  end_text();

  val = gtk_spin_button_get_value_as_int(spinbutton) - 1;

  if (val == journal.npages) { // create a page at end
    on_journalNewPageEnd_activate(NULL, NULL);
    return;
  }

  if (val == ui.pageno) return;
  if (val < 0) val = 0;
  if (val > journal.npages-1) val = journal.npages-1;
  do_switch_page(val, TRUE, FALSE);
}


void
on_journalDefaultBackground_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  struct Page *pg;
  GList *pglist;
  
  end_text();
  reset_selection();
  
  pg = ui.cur_page;
  for (pglist = journal.pages; pglist!=NULL; pglist = pglist->next) {
    if (ui.bg_apply_all_pages) pg = (struct Page *)pglist->data;
    prepare_new_undo();
    if (ui.bg_apply_all_pages) {
      if (pglist->next!=NULL) undo->multiop |= MULTIOP_CONT_REDO;
      if (pglist->prev!=NULL) undo->multiop |= MULTIOP_CONT_UNDO;
    }
    undo->type = ITEM_NEW_BG_RESIZE;
    undo->page = pg;
    undo->bg = pg->bg;
    undo->val_x = pg->width;
    undo->val_y = pg->height; 
    pg->bg = (struct Background *)g_memdup(ui.default_page.bg, sizeof(struct Background));
    pg->width = ui.default_page.width;
    pg->height = ui.default_page.height;
    pg->bg->canvas_item = undo->bg->canvas_item;
    undo->bg->canvas_item = NULL;
  
    make_page_clipbox(pg);
    update_canvas_bg(pg);
    if (!ui.bg_apply_all_pages) break;
  }
  do_switch_page(ui.pageno, TRUE, TRUE);
}


void
on_journalSetAsDefault_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (ui.cur_page->bg->type != BG_SOLID) return;
  
  end_text();
  prepare_new_undo();
  undo->type = ITEM_NEW_DEFAULT_BG;
  undo->val_x = ui.default_page.width;
  undo->val_y = ui.default_page.height;
  undo->bg = ui.default_page.bg;
  
  ui.default_page.width = ui.cur_page->width;
  ui.default_page.height = ui.cur_page->height;
  ui.default_page.bg = (struct Background *)g_memdup(ui.cur_page->bg, sizeof(struct Background));
  ui.default_page.bg->canvas_item = NULL;
}


void
on_comboStdSizes_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
  GtkEntry *entry;
  GtkComboBox *comboUnit;
  int val;
  gchar text[20];

  if (papersize_need_init) {
    gtk_combo_box_set_active(combobox, papersize_std);
    papersize_need_init = FALSE;
  } else {
    val = gtk_combo_box_get_active(combobox);
    if (val == -1 || val == papersize_std) return;
    papersize_std = val;
    if (val == STD_SIZE_CUSTOM) return;
    papersize_unit = std_units[val];
    papersize_width = std_widths[val];
    papersize_height = std_heights[val];
  }
  comboUnit = GTK_COMBO_BOX(g_object_get_data(G_OBJECT(papersize_dialog), "comboUnit"));
  gtk_combo_box_set_active(comboUnit, papersize_unit);
  entry = GTK_ENTRY(g_object_get_data(G_OBJECT(papersize_dialog), "entryWidth"));
  g_snprintf(text, 20, "%.2f", papersize_width/unit_sizes[papersize_unit]);
  if (g_str_has_suffix(text, ".00")) 
    g_snprintf(text, 20, "%d", (int) (papersize_width/unit_sizes[papersize_unit]));
  gtk_entry_set_text(entry, text);
  entry = GTK_ENTRY(g_object_get_data(G_OBJECT(papersize_dialog), "entryHeight"));
  g_snprintf(text, 20, "%.2f", papersize_height/unit_sizes[papersize_unit]);
  if (g_str_has_suffix(text, ".00")) 
    g_snprintf(text, 20, "%d", (int) (papersize_height/unit_sizes[papersize_unit]));
  gtk_entry_set_text(entry, text);
}


void
on_entryWidth_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
  double val;
  const gchar *text;
  gchar *ptr;
  GtkComboBox *comboStdSizes;
  
  text = gtk_entry_get_text(GTK_ENTRY(editable));
  val = strtod(text, &ptr);
  papersize_width_valid = (*ptr == 0 && val > 0.);
  if (!papersize_width_valid) return; // invalid entry
  val *= unit_sizes[papersize_unit];
  if (fabs(val - papersize_width) < 0.1) return; // no change
  papersize_std = STD_SIZE_CUSTOM;
  papersize_width = val;
  comboStdSizes = GTK_COMBO_BOX(g_object_get_data(G_OBJECT(papersize_dialog), "comboStdSizes"));
  gtk_combo_box_set_active(comboStdSizes, papersize_std);
}


void
on_entryHeight_changed                 (GtkEditable     *editable,
                                        gpointer         user_data)
{
  double val;
  const gchar *text;
  gchar *ptr;
  GtkComboBox *comboStdSizes;
  
  text = gtk_entry_get_text(GTK_ENTRY(editable));
  val = strtod(text, &ptr);
  papersize_height_valid = (*ptr == 0 && val > 0.);
  if (!papersize_height_valid) return; // invalid entry
  val *= unit_sizes[papersize_unit];
  if (fabs(val - papersize_height) < 0.1) return; // no change
  papersize_std = STD_SIZE_CUSTOM;
  papersize_height = val;
  comboStdSizes = GTK_COMBO_BOX(g_object_get_data(G_OBJECT(papersize_dialog), "comboStdSizes"));
  gtk_combo_box_set_active(comboStdSizes, papersize_std);
}


void
on_comboUnit_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
  GtkEntry *entry;
  int val;
  gchar text[20];

  val = gtk_combo_box_get_active(combobox);
  if (val == -1 || val == papersize_unit) return;
  papersize_unit = val;
  entry = GTK_ENTRY(g_object_get_data(G_OBJECT(papersize_dialog), "entryWidth"));
  if (papersize_width_valid) {
    g_snprintf(text, 20, "%.2f", papersize_width/unit_sizes[papersize_unit]);
    if (g_str_has_suffix(text, ".00")) 
      g_snprintf(text, 20, "%d", (int) (papersize_width/unit_sizes[papersize_unit]));
  } else *text = 0;
  gtk_entry_set_text(entry, text);
  if (papersize_height_valid) {
    entry = GTK_ENTRY(g_object_get_data(G_OBJECT(papersize_dialog), "entryHeight"));
    g_snprintf(text, 20, "%.2f", papersize_height/unit_sizes[papersize_unit]);
    if (g_str_has_suffix(text, ".00")) 
      g_snprintf(text, 20, "%d", (int) (papersize_height/unit_sizes[papersize_unit]));
  } else *text = 0;
  gtk_entry_set_text(entry, text);
}


void
on_viewFullscreen_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gboolean active;
  
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_CHECK_MENU_ITEM)
    active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  else
    active = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem));

#ifndef USE_HILDON
  if (active == ui.fullscreen) return;
#else
  ui.fullscreen = !ui.fullscreen;
  active = ui.fullscreen;
#endif

  do_fullscreen(active);
}


void
on_optionsButtonMappings_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  ui.use_erasertip =
    gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  update_mappings_menu();
}


void
on_optionsProgressiveBG_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gboolean active;
  
  active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  if (ui.progressive_bg == active) return;
  end_text();
  ui.progressive_bg = active;
  if (!ui.progressive_bg) rescale_bg_pixmaps();
}


void
on_mru_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int which;
  gboolean success;
  GtkWidget *dialog;
  
  end_text();
  if (!ok_to_close()) return; // user aborted on save confirmation
  
  for (which = 0 ; which < MRU_SIZE; which++) {
    if (ui.mrumenu[which] == GTK_WIDGET(menuitem)) break;
  }
  if (which == MRU_SIZE || ui.mru[which] == NULL) return; // not found...

  set_cursor_busy(TRUE);
#ifdef USE_HILDON
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 1);
#endif
  success = open_journal(ui.mru[which]);
#ifdef USE_HILDON
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW(winMain), 0);
#endif
  set_cursor_busy(FALSE);
  if (success) return;

  /* open failed */
#ifdef USE_HILDON
  {
    gchar descr[strlen(_("Error opening file ''"))+ strlen(ui.mru[which])];
    sprintf (descr, _("Error opening file '%s'"), ui.mru[which]);
    dialog = hildon_note_new_information (GTK_WINDOW (winMain), descr);
  }
#else
  dialog = gtk_message_dialog_new(GTK_WINDOW (winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error opening file '%s'"), ui.mru[which]);
#endif
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  delete_mru_entry(which);
}


void
on_button2Pen_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_PEN);
}


void
on_button2Eraser_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_ERASER);
}


void
on_button2Highlighter_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_HIGHLIGHTER);
}


void
on_button2Text_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_TEXT);
}


void
on_button2SelectRegion_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_SELECTREGION);
}


void
on_button2SelectRectangle_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_SELECTRECT);
}


void
on_button2VerticalSpace_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_VERTSPACE);
}


void
on_button2LinkBrush_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int i;
  
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) return;
  end_text();
  ui.linked_brush[1] = BRUSH_LINKED;
  for (i=0;i<NUM_STROKE_TOOLS;i++) update_mapping_linkings(i);
}


void
on_button2CopyBrush_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) return;
  end_text();
  if (ui.toolno[1] >= NUM_STROKE_TOOLS) {
    ui.linked_brush[1] = BRUSH_STATIC;
    update_mappings_menu_linkings();
    return;
  }
  ui.linked_brush[1] = BRUSH_COPIED;
  g_memmove(&(ui.brushes[1][ui.toolno[1]]), &(ui.brushes[0][ui.toolno[1]]), sizeof(struct Brush));
}


void
on_button3Pen_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_PEN);
}


void
on_button3Eraser_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_ERASER);
}


void
on_button3Highlighter_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_HIGHLIGHTER);
}


void
on_button3Text_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_TEXT);
}


void
on_button3SelectRegion_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_SELECTREGION);
}


void
on_button3SelectRectangle_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_SELECTRECT);
}


void
on_button3VerticalSpace_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_VERTSPACE);
}


void
on_button3LinkBrush_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int i;
  
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) return;
  end_text();
  ui.linked_brush[2] = BRUSH_LINKED;
  for (i=0;i<NUM_STROKE_TOOLS;i++) update_mapping_linkings(i);
}


void
on_button3CopyBrush_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) return;
  end_text();
  if (ui.toolno[2] >= NUM_STROKE_TOOLS) {
    ui.linked_brush[2] = BRUSH_STATIC;
    update_mappings_menu_linkings();
    return;
  }
  ui.linked_brush[2] = BRUSH_COPIED;
  g_memmove(&(ui.brushes[2][ui.toolno[2]]), &(ui.brushes[0][ui.toolno[2]]), sizeof(struct Brush));
}

// the set zoom dialog

GtkWidget *zoom_dialog;
double zoom_percent;

void
on_viewSetZoom_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int response;
  double test_w, test_h;
  GtkSpinButton *spinZoom;
  
  end_text();
  zoom_dialog = create_zoomDialog();
  zoom_percent = 100*ui.zoom / DEFAULT_ZOOM;
  spinZoom = GTK_SPIN_BUTTON(g_object_get_data(G_OBJECT(zoom_dialog), "spinZoom"));
  gtk_spin_button_set_increments(spinZoom, ui.zoom_step_increment, 5*ui.zoom_step_increment);
  gtk_spin_button_set_value(spinZoom, zoom_percent);
  test_w = 100*(GTK_WIDGET(canvas))->allocation.width/ui.cur_page->width/DEFAULT_ZOOM;
  test_h = 100*(GTK_WIDGET(canvas))->allocation.height/ui.cur_page->height/DEFAULT_ZOOM;
  if (zoom_percent > 99.9 && zoom_percent < 100.1) 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_object_get_data(
           G_OBJECT(zoom_dialog), "radioZoom100")), TRUE);
  else if (zoom_percent > test_w-0.1 && zoom_percent < test_w+0.1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_object_get_data(
           G_OBJECT(zoom_dialog), "radioZoomWidth")), TRUE);
  else if (zoom_percent > test_h-0.1 && zoom_percent < test_h+0.1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_object_get_data(
           G_OBJECT(zoom_dialog), "radioZoomHeight")), TRUE);
  else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_object_get_data(
           G_OBJECT(zoom_dialog), "radioZoom")), TRUE);
  gtk_widget_show(zoom_dialog);
  
  do {
    response = gtk_dialog_run(GTK_DIALOG(zoom_dialog));
    if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY) {
      ui.zoom = DEFAULT_ZOOM*zoom_percent/100;
      gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
      rescale_text_items();
      rescale_bg_pixmaps();
    }
  } while (response == GTK_RESPONSE_APPLY);
  
  gtk_widget_destroy(zoom_dialog);
}


void
on_spinZoom_value_changed              (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  double val;

  val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(g_object_get_data(
             G_OBJECT(zoom_dialog), "spinZoom")));
  if (val<1) return;
  if (val<10) val=10.;
  if (val>1500) val=1500.;
  if (val<zoom_percent-1 || val>zoom_percent+1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_object_get_data(
           G_OBJECT(zoom_dialog), "radioZoom")), TRUE);
  zoom_percent = val;
}


void
on_radioZoom_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  // nothing to do
}


void
on_radioZoom100_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!gtk_toggle_button_get_active(togglebutton)) return;
  zoom_percent = 100.;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(g_object_get_data(
        G_OBJECT(zoom_dialog), "spinZoom")), zoom_percent);
}


void
on_radioZoomWidth_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!gtk_toggle_button_get_active(togglebutton)) return;
  zoom_percent = 100*(GTK_WIDGET(canvas))->allocation.width/ui.cur_page->width/DEFAULT_ZOOM;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(g_object_get_data(
        G_OBJECT(zoom_dialog), "spinZoom")), zoom_percent);
}


void
on_radioZoomHeight_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!gtk_toggle_button_get_active(togglebutton)) return;
  zoom_percent = 100*(GTK_WIDGET(canvas))->allocation.height/ui.cur_page->height/DEFAULT_ZOOM;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(g_object_get_data(
        G_OBJECT(zoom_dialog), "spinZoom")), zoom_percent);
}


void
on_toolsHand_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (menuitem) {
  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_RADIO_MENU_ITEM) {
    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem)))
      return;
  } else {
    if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem)))
      return;
  }
  }

  if (ui.cur_mapping != 0 && !ui.button_switch_mapping) return;
  if (ui.toolno[ui.cur_mapping] == TOOL_HAND) return;

  ui.cur_mapping = 0;
  end_text();
  reset_selection();
  ui.toolno[ui.cur_mapping] = TOOL_HAND;
  update_mapping_linkings(-1);
  update_tool_buttons();
  update_tool_menu();
  update_color_menu();
  update_cursor();
}


void
on_button2Hand_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 1, TOOL_HAND);
}


void
on_button3Hand_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  process_mapping_activate(menuitem, 2, TOOL_HAND);
}


void
on_optionsPrintRuling_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  ui.print_ruling = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
}

void
on_optionsAutoloadPdfXoj_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  end_text();
  ui.autoload_pdf_xoj = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
}

void
on_fontButton_font_set                 (GtkFontButton   *fontbutton,
                                        gpointer         user_data)
{
  gchar *str;
  
  str = g_strdup(gtk_font_button_get_font_name(fontbutton));
  process_font_sel(str);
}

void
on_optionsLeftHanded_activate          (GtkMenuItem     *menuitem,   
                                        gpointer         user_data)
{
  end_text();
  ui.left_handed = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(GET_COMPONENT("scrolledwindowMain")),
    ui.left_handed?GTK_CORNER_TOP_RIGHT:GTK_CORNER_TOP_LEFT);
}

void
on_optionsShortenMenus_activate        (GtkMenuItem     *menuitem,  
                                        gpointer         user_data)
{
  gchar *item, *nextptr;
  GtkWidget *w;
  
  end_text();
  ui.shorten_menus = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  
  /* go over the item list */
  item = ui.shorten_menu_items;
  while (*item==' ') item++;
  while (*item) {
    nextptr = strchr(item, ' ');
    if (nextptr!=NULL) *nextptr = 0;
    // hide or show the item
    w = GET_COMPONENT(item);
    if (w != NULL) {
      if (ui.shorten_menus) gtk_widget_hide(w);
      else gtk_widget_show(w);
    }
    // next item
    if (nextptr==NULL) break;
    *nextptr = ' ';
    item = nextptr;
    while (*item==' ') item++;
  }
  
  // just in case someone tried to unhide stuff they shouldn't be seeing
  hide_unimplemented();
  // maybe we should also make sure the drawing area stays visible ?
}

void
on_optionsAutoSavePrefs_activate       (GtkMenuItem     *menuitem,  
                                        gpointer         user_data)
{
  end_text();
  ui.auto_save_prefs = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
}

void
on_optionsPressureSensitive_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  int i;

#ifdef USE_HILDON
  gboolean active;

  if (GTK_OBJECT_TYPE(menuitem) == GTK_TYPE_CHECK_MENU_ITEM)
    active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
  else
    active = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON (menuitem));
#endif

  end_text();

#ifdef USE_HILDON
  ui.pressure_sensitivity = active;
#else
  ui.pressure_sensitivity = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
#endif
  printf ("Pressure sens is %d\n", ui.pressure_sensitivity);
  for (i=0; i<=NUM_BUTTONS; i++)
    ui.brushes[i][TOOL_PEN].variable_width = ui.pressure_sensitivity;
  update_mappings_menu();
#ifdef USE_HILDON
  update_tool_buttons();
#endif
}


void
on_buttonColorChooser_set              (GtkColorButton  *colorbutton,
                                        gpointer         user_data)
{
  GdkColor gdkcolor;
  guint16 alpha;
  
  gtk_color_button_get_color(colorbutton, &gdkcolor);
  alpha = gtk_color_button_get_alpha(colorbutton);
  process_color_activate((GtkMenuItem*)colorbutton, COLOR_OTHER, gdkcolor_to_rgba(gdkcolor, alpha));
}


void
on_optionsButtonsSwitchMappings_activate(GtkMenuItem    *menuitem,
                                        gpointer         user_data)
{
  end_text();
  switch_mapping(0);
  ui.button_switch_mapping = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (menuitem));
}

void hildon_tools_other_action (GtkAction *action)
{
	switch (gtk_radio_action_get_current_value(GTK_RADIO_ACTION (action))) {
		case 0:
			on_toolsReco_activate (NULL, NULL);
			break;
		case 1:
			on_toolsRuler_activate (NULL, NULL);
			break;
	}
}

void hildon_tools_radio_action (GtkAction *action)
{
	GtkWidget *ruler_button;

	switch (gtk_radio_action_get_current_value(GTK_RADIO_ACTION (action))) {
		case 0:
  			ruler_button = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBar/Ruler");
      			gtk_toggle_tool_button_set_active(
 			       GTK_TOGGLE_TOOL_BUTTON(ruler_button), FALSE);
			on_toolsPen_activate (NULL, NULL);
			break;
		case 1:
  			ruler_button = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBar/Ruler");
      			gtk_toggle_tool_button_set_active(
 			       GTK_TOGGLE_TOOL_BUTTON(ruler_button), FALSE);
			on_toolsEraser_activate (NULL, NULL);
			break;
		case 2:
  			ruler_button = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBar/Ruler");
      			gtk_toggle_tool_button_set_active(
 			       GTK_TOGGLE_TOOL_BUTTON(ruler_button), FALSE);
			on_toolsHighlighter_activate (NULL, NULL);
			break;
		case 3:
  			ruler_button = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBar/Ruler");
      			gtk_toggle_tool_button_set_active(
 			       GTK_TOGGLE_TOOL_BUTTON(ruler_button), FALSE);
			on_toolsText_activate (NULL, NULL);
			break;
		case 4:
  			ruler_button = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBar/Ruler");
      			gtk_toggle_tool_button_set_active(
 			       GTK_TOGGLE_TOOL_BUTTON(ruler_button), FALSE);
			on_toolsSelectRegion_activate (NULL, NULL);
			break;
		case 5:
  			ruler_button = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBar/Ruler");
      			gtk_toggle_tool_button_set_active(
 			       GTK_TOGGLE_TOOL_BUTTON(ruler_button), FALSE);
			on_toolsVerticalSpace_activate (NULL, NULL);
			break;
		case 6:
  			ruler_button = gtk_ui_manager_get_widget (ui_manager, "/HildonToolBar/Ruler");
      			gtk_toggle_tool_button_set_active(
 			       GTK_TOGGLE_TOOL_BUTTON(ruler_button), FALSE);
			on_toolsHand_activate (NULL, NULL);
			break;
	}
}

void
hildon_show_view (GtkWidget *item, gpointer user_data)
{
	GtkWidget *new_view = GTK_WIDGET(user_data);
	HildonWindowStack *stack;

	stack = hildon_window_stack_get_default ();

	/* pop current view from the stack */
	hildon_window_stack_pop_1 (stack);

	/* push new view on top of the stack */
	hildon_window_stack_push_1 (stack, HILDON_STACKABLE_WINDOW(new_view));
}

void
on_toolStackableWindow (void)
{
	HildonAppMenu *stack_menu;

	tools_view = hildon_create_tools_view ();
	settings_view = hildon_create_settings_view ();
	journal_view = hildon_create_journal_view ();

	stack_menu = hildon_create_stack_menu ();
	hildon_window_set_app_menu (HILDON_WINDOW(tools_view), HILDON_APP_MENU(stack_menu));
	hildon_window_set_app_menu (HILDON_WINDOW(journal_view), HILDON_APP_MENU(stack_menu));
	hildon_window_set_app_menu (HILDON_WINDOW(settings_view), HILDON_APP_MENU(stack_menu));

	gtk_widget_show_all (tools_view);
}

void
hildon_share_via_email (GtkWidget *item, gpointer user_data)
{
	GSList *list = NULL;
	gboolean result = FALSE;

	if (!ui.filename_pdf) {
		gtk_widget_destroy (GTK_WIDGET(user_data));
		return;
	}

	list = g_slist_append(list, ui.filename_pdf);
	result = libmodest_dbus_client_compose_mail(ctx, /*osso_context_t*/
		NULL, /*to*/
		NULL, /*cc*/
		NULL, /*bcc*/
		NULL, /*body*/
		NULL, /*subject*/
		list /*attachments*/);

	g_slist_free(list);

	gtk_widget_destroy (GTK_WIDGET(user_data));
}

void
hildon_share_via_bt (GtkWidget *item, gpointer user_data)
{
	DBusGProxy *proxy = NULL;
	DBusGConnection *sys_conn = NULL;
	GError *error = NULL;
	gchar **files = NULL;

	if (!ui.filename_pdf) {
		gtk_widget_destroy (GTK_WIDGET(user_data));
		return;
	}

	/*sys_conn = osso_get_sys_dbus_connection(ctx);*/
	sys_conn = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
		
	if(!sys_conn) {
		g_print("Error connecting to DBUS\n");
		gtk_widget_destroy (GTK_WIDGET(user_data));
		return;
	}

	files = g_new0(gchar*, 2);
	*files = g_strdup(ui.filename_pdf);
	files[1] = NULL;
	
	/* Open connection for btdialogs service */
	proxy = dbus_g_proxy_new_for_name(sys_conn,
		CONBTDIALOGS_DBUS_SERVICE,
		CONBTDIALOGS_DBUS_PATH,
		CONBTDIALOGS_DBUS_INTERFACE);

	/* Send file request to btdialogs service */
	if (!dbus_g_proxy_call(proxy, CONBTDIALOGS_SEND_FILE_REQ,
		&error, G_TYPE_STRV, files,
		G_TYPE_INVALID, G_TYPE_INVALID))
	{
		g_print("Error: %s\n", error->message);
		g_clear_error (&error);
	}

	g_strfreev(files);

	g_object_unref(proxy);

	gtk_widget_destroy (GTK_WIDGET(user_data));
}

void
hildon_share_tool (GtkMenuItem  *menuitem, gpointer user_data)
{
	GtkWidget *banner;
	static void *dl_handle = NULL;
	static void (*sharing_dialog_with_file) (osso_context_t *, GtkWidget *, const gchar *);
	gchar *error;

	banner=hildon_banner_show_information (GTK_WIDGET(winMain), NULL, _("File must be saved as PDF first..."));
	hildon_banner_set_timeout (HILDON_BANNER(banner), 2000);

	gtk_widget_destroy (banner);

	on_filePrintPDF_activate (NULL, NULL);

	if (!ui.filename_pdf) {
		return;
	}

	if (!dl_handle) {
		dl_handle = dlopen (HILDON_LIBSHARINGDIALOG_LIB,
				RTLD_LAZY);
		if (!dl_handle) {
			g_printerr ("Error: %s\n", dlerror());

			return;
		}

		*(void **)(&sharing_dialog_with_file) = dlsym (dl_handle,
			"sharing_dialog_with_file");
		if ((error = dlerror()) != NULL) {
			g_printerr ("Error: %s\n", dlerror());
			dlclose (dl_handle);

			return;
		}
	}

	banner=hildon_banner_show_information (GTK_WIDGET(winMain), NULL, _("Select how do you want to share the note"));
	hildon_banner_set_timeout (HILDON_BANNER(banner), 2000);

	gtk_widget_destroy (banner);

	sharing_dialog_with_file (ctx,
			winMain,
			ui.filename_pdf);
}

#if 0
void
hildon_share_tool (GtkMenuItem  *menuitem, gpointer user_data)
{
	GtkWidget *dialog;
	GtkWidget *content_area, *button_box;
	GtkWidget *email_button, *bt_button, *service_button;
	GtkWidget *banner;
	gchar title[100];
	gint  result;

	banner=hildon_banner_show_information (GTK_WIDGET(winMain), NULL, _("File must be saved as PDF first..."));
	hildon_banner_set_timeout (HILDON_BANNER(banner), 2000);

	on_filePrintPDF_activate (NULL, NULL);

	if (!ui.filename_pdf) {
		return;
	}

	banner=hildon_banner_show_information (GTK_WIDGET(winMain), NULL, _("Select how do you want to share the note"));
	hildon_banner_set_timeout (HILDON_BANNER(banner), 2000);

        g_sprintf(title, _("Share %s"), g_basename(ui.filename_pdf));

	dialog = gtk_dialog_new_with_buttons (title,
			GTK_WINDOW(winMain),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			NULL);

	content_area = gtk_dialog_get_content_area (GTK_DIALOG(dialog));

	button_box = gtk_hbox_new (TRUE, 10);
	gtk_widget_show(button_box);

	email_button = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_label(GTK_BUTTON(email_button), ("Via e-mail"));
//	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(email_button), FALSE);
	gtk_widget_show (email_button);
	gtk_box_pack_start (GTK_BOX(button_box), email_button, TRUE, TRUE, 0);
	g_signal_connect (email_button, "clicked", G_CALLBACK(hildon_share_via_email), dialog);

	bt_button = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_label(GTK_BUTTON(bt_button), _("Via bluetooth"));
//	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(bt_button), FALSE);
	gtk_widget_show (bt_button);
	gtk_box_pack_start (GTK_BOX(button_box), bt_button, TRUE, TRUE, 0);
	g_signal_connect (bt_button, "clicked", G_CALLBACK(hildon_share_via_bt), dialog);

	// let's wait for Nokia to disclose APIs for the share via service functionality
	//
	service_button = hildon_gtk_button_new(HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_label(GTK_BUTTON(service_button), _("Via service"));
//	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(service_button), FALSE);
	gtk_widget_show (service_button);
	gtk_box_pack_start (GTK_BOX(button_box), service_button, TRUE, TRUE, 0);
	g_signal_connect (service_button, "clicked", G_CALLBACK(hildon_share_via_service), dialog);

	gtk_box_pack_start (GTK_BOX(content_area), button_box, TRUE, TRUE, 0);

	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}
#endif