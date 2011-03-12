#ifndef _XO_HILDON_UI_H
#define _XO_HILDON_UI_H
#include <gtk/gtk.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <hildon/hildon.h>
#include <hildon/hildon-defines.h>
#include <hildon/hildon-window.h>

#include <libosso.h>

#include "xournal.h"
#include "hildon-extras/he-color-button.h"
#include "xo-callbacks.h"

#define HILDON_NUM_TOOLBARS		12

extern char *hildonToolbars[HILDON_NUM_TOOLBARS];

enum
{
  HILDON_TOOLBAR_NOTE_LANDSCAPE,
  HILDON_TOOLBAR_NOTE_PORTRAIT,
  HILDON_TOOLBAR_PDF_LANDSCAPE,
  HILDON_TOOLBAR_PDF_PORTRAIT,
  HILDON_TOOLBAR_DRAW_LANDSCAPE,
  HILDON_TOOLBAR_DRAW_PORTRAIT,
  HILDON_TOOLBAR_HAND_LANDSCAPE,
  HILDON_TOOLBAR_HAND_PORTRAIT,
  HILDON_TOOLBAR_PEN_LANDSCAPE,
  HILDON_TOOLBAR_PEN_PORTRAIT,
  HILDON_TOOLBAR_COLOR_LANDSCAPE,
  HILDON_TOOLBAR_COLOR_PORTRAIT
};

extern osso_context_t *ctx;

GtkWidget *hildon_undo[HILDON_NUM_TOOLBARS];
GtkWidget *hildon_redo[HILDON_NUM_TOOLBARS];

GtkWidget *hildon_toolbars[HILDON_NUM_TOOLBARS];
GtkWidget *hildon_cur_toolbar_landscape;
GtkWidget *hildon_cur_toolbar_portrait;
GtkWidget *switch_to_toolbar_landscape;
GtkWidget *switch_to_toolbar_portrait;

//HildonColorButton *buttonColor;
HeColorButton *buttonColor;
GtkWidget *pickerButton;

#if 0
// Two & 1/2 button click
gint hildon_button_pressed_timedout (gpointer data);
extern gint two_half_button_click,
       hildon_button_pressed,
       hildon_button_released,
       hildon_timer_running_already;
#endif

GtkWidget* create_winMain (osso_context_t *);
GtkWidget* create_papersizeDialog (void);
GtkWidget* create_aboutDialog (void);
GtkWidget* create_zoomDialog (void);

HildonAppMenu *hildon_create_stack_menu (void);
GtkWidget *hildon_create_tools_view (void);
GtkWidget *hildon_create_journal_view (void);
GtkWidget *hildon_create_settings_view (void);
gboolean device_is_portrait_mode(osso_context_t *);

extern GtkWidget *tools_view, *settings_view, *journal_view;
void hildon_show_view (GtkWidget *, gpointer);
void hildon_grab_volume_keys (int);
DBusHandlerResult mce_filter_func (DBusConnection *, DBusMessage *, void *);

void hildon_draw_switch (GtkWidget *, gpointer);
void hildon_pdf_switch (GtkWidget *, gpointer);
void hildon_note_switch (GtkWidget *, gpointer);
void hildon_toolbar_switch (GtkWidget *, gpointer);

void hildon_pen_activate (GtkWidget *, gpointer);
void hildon_eraser_activate (GtkWidget *, gpointer);
void hildon_highlighter_activate (GtkWidget *, gpointer);
void hildon_zoom_activate (GtkWidget *, gpointer);
void hildon_layer_activate (GtkWidget *, gpointer);
void hildon_page_activate (GtkWidget *, gpointer);
void hildon_paper_activate (GtkWidget *, gpointer);

/* Menu Actions */
static GtkActionEntry hildon_menu_entries[] = {
  { "HildonMainMenu", NULL, N_("_Xournal") },
  { "FileMenu", NULL, N_("_Xournal") },
  { "EditMenu", NULL, N_("_Edit") },
  { "ViewMenu", NULL, N_("_View") },
  { "JournalMenu", NULL, N_("_Journal") },
  { "ToolsMenu", NULL, N_("_Tools") },
  { "OptionsMenu", NULL, N_("_Options") },
  { "fileRecentFiles", NULL, N_("Recent doc_uments") },
  { "ZoomMenu", NULL, N_("_Zoom") },
  { "journalPaperColor", NULL, N_("Paper _color") },
  { "journalPaperStyle", NULL, N_("Paper _style") },
  { "PenOptionsMenu", NULL, N_("_Pen options") },
  { "EraserOptionsMenu", NULL, N_("_Eraser _options") },
  { "HighlighterOptionsMenu", NULL, N_("_Highlighter options") },
  { "mru0", NULL, "0", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "mru1", NULL, "1", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "mru2", NULL, "2", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "mru3", NULL, "3", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "mru4", NULL, "4", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "mru5", NULL, "5", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "mru6", NULL, "6", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "mru7", NULL, "7", NULL, NULL, G_CALLBACK(on_mru_activate) },
  { "New", GTK_STOCK_NEW, N_("_New"), "<control>N", NULL, G_CALLBACK(on_fileNew_activate) },
  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O", NULL, G_CALLBACK(on_fileOpen_activate) },
  { "OpenPDF", GTK_STOCK_OPEN, N_("Annotate PD_F..."), NULL, NULL, G_CALLBACK(on_fileNewBackground_activate) },
  { "Save", GTK_STOCK_SAVE, N_("_Save"), "<control>S", NULL, G_CALLBACK(on_fileSave_activate) },
  { "SaveAs", GTK_STOCK_SAVE, N_("Save _as"), NULL, NULL, G_CALLBACK(on_fileSaveAs_activate) },
  { "ExportPDF", GTK_STOCK_SAVE, N_("_Export to PDF"), NULL, NULL, G_CALLBACK(on_filePrintPDF_activate) },
  { "Undo", "general_undo", N_("_Undo"), "<control>Z", NULL, G_CALLBACK(on_editUndo_activate) },
  { "Redo", "general_redo", N_("_Redo"), "<control>Y", NULL, G_CALLBACK(on_editRedo_activate) },
  { "Cut", GTK_STOCK_CUT, N_("_Cu_t"), "<control>X", NULL, G_CALLBACK(on_editCut_activate) },
  { "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C", NULL, G_CALLBACK(on_editCopy_activate) },
  { "Paste", GTK_STOCK_PASTE, N_("_Paste"), "<control>V", NULL, G_CALLBACK(on_editPaste_activate) },
  { "Delete", GTK_STOCK_DELETE, N_("_Delete"), "backspace", NULL, G_CALLBACK(on_editDelete_activate) },
  { "ZoomIn", GTK_STOCK_ZOOM_IN, N_("Zoom _In"), "plus", NULL, G_CALLBACK(on_viewZoomIn_activate) },
  { "ZoomOut", GTK_STOCK_ZOOM_OUT, N_("Zoom _Out"), "minus", NULL, G_CALLBACK(on_viewZoomOut_activate) },
  { "NormalSize", GTK_STOCK_ZOOM_100, N_("_Normal size"), "Z", NULL, G_CALLBACK(on_viewNormalSize_activate) },
  { "PageWidth", GTK_STOCK_ZOOM_FIT, N_("Page _Width"), "<control><shift>0", NULL, G_CALLBACK(on_viewPageWidth_activate) },
  { "SetZoom", NULL, N_("_Set zoom"), NULL, NULL, G_CALLBACK(on_viewSetZoom_activate) },
  { "FirstPage", GTK_STOCK_GOTO_FIRST, N_("_First page"), NULL, NULL, G_CALLBACK(on_viewFirstPage_activate) },
  { "PreviousPage", "hi_xo_prev", N_("_Previous page"), NULL, NULL, G_CALLBACK(on_viewPreviousPage_activate) },
  { "NextPage", "hi_xo_next", N_("_Next page"), NULL, NULL, G_CALLBACK(on_viewNextPage_activate) },
  { "LastPage", GTK_STOCK_GOTO_LAST, N_("_Last page"), NULL, NULL, G_CALLBACK(on_viewLastPage_activate) },
  { "ShowLayers", GTK_STOCK_REMOVE, N_("Select _layer"), NULL, NULL, G_CALLBACK(on_HildonShowLayer_activate) },
  { "ShowLayer", GTK_STOCK_ADD, N_("_Show layer"), NULL, NULL, G_CALLBACK(on_viewShowLayer_activate) },
  { "HideLayer", GTK_STOCK_REMOVE, N_("_Hide layer"), NULL, NULL, G_CALLBACK(on_viewHideLayer_activate) },
  { "NewPageBefore", "hi_xo_newpagebefore", N_("New page _before"), NULL, NULL, G_CALLBACK(on_journalNewPageBefore_activate) },
  { "NewPageAfter", "hi_xo_newpageafter", N_("New page _after"), NULL, NULL, G_CALLBACK(on_journalNewPageAfter_activate) },
  { "NewPageEnd", "hi_xo_newpageafter", N_("New page at _end"), NULL, NULL, G_CALLBACK(on_journalNewPageEnd_activate) },
  { "DeletePage", "hi_xo_deletepage", N_("_Delete page"), NULL, NULL, G_CALLBACK(on_journalDeletePage_activate) },
  { "NewLayer", NULL, N_("_New layer"), NULL, NULL, G_CALLBACK(on_journalNewLayer_activate) },
  { "DeleteLayer", NULL, N_("_Delete layer"), NULL, NULL, G_CALLBACK(on_journalDeleteLayer_activate) },
  { "PaperSize", NULL, N_("Paper si_ze"), NULL, NULL, G_CALLBACK(on_journalPaperSize_activate) },
  { "LoadBackground", GTK_STOCK_OPEN, N_("_Load background"), NULL, NULL, G_CALLBACK(on_journalLoadBackground_activate) },
  { "Screenshot", NULL, N_("Background screens_hot"), "<control><shift>S", NULL, G_CALLBACK(on_journalScreenshot_activate) },
  { "DefaultBackground", NULL, N_("Default _paper"), NULL, NULL, G_CALLBACK(on_journalDefaultBackground_activate) },
  { "journalSetAsDefault", NULL, N_("Set as de_fault"), NULL, NULL, G_CALLBACK(on_journalSetAsDefault_activate) },
  { "TextFont", GTK_STOCK_SELECT_FONT, N_("Text _font"), "<control><shift>F", NULL, G_CALLBACK(on_toolsTextFont_activate) },
  { "DefaultPen", NULL, N_("Default _Pen"), NULL, NULL, G_CALLBACK(on_toolsDefaultPen_activate) },
  { "DefaultEraser", NULL, N_("Default _Eraser"), NULL, NULL, G_CALLBACK(on_toolsDefaultEraser_activate) },
  { "DefaultHighlighter", NULL, N_("Default _Highlighter"), NULL, NULL, G_CALLBACK(on_toolsDefaultHighlighter_activate) },
  { "DefaultText", NULL, N_("_Default _Text"), NULL, NULL, G_CALLBACK(on_toolsDefaultText_activate) },
  { "toolsSetAsDefault", NULL, N_("Set as default"), NULL, NULL, G_CALLBACK(on_toolsSetAsDefault_activate) },
  { "Pressure", "hi_xo_pressure", N_("_Pressure sensitivity"), NULL, NULL, G_CALLBACK(on_optionsPressureSensitive_activate) },
  { "ProgressiveBG",NULL, N_("_Progressive backgrounds"), NULL, NULL, G_CALLBACK(on_optionsProgressiveBG_activate) },
  { "PrintRuling", NULL, N_("_Print paper _ruling"), NULL, NULL, G_CALLBACK(on_optionsPrintRuling_activate) },
  { "LeftHanded", NULL, N_("Left-Handed scrollbar"), NULL, NULL, G_CALLBACK(on_optionsLeftHanded_activate) },
  { "AutoSavePrefs", NULL, N_("A_uto-save preferences"), NULL, NULL, G_CALLBACK(on_optionsAutoSavePrefs_activate) },
  { "SavePreferences", NULL, N_("_Save preferences"), NULL, NULL, G_CALLBACK(on_optionsSavePreferences_activate) },
  { "Help", NULL, N_("_Help..."), NULL, NULL, G_CALLBACK(on_HildonHelp_activate) },
  { "About", NULL, N_("_About..."), NULL, NULL, G_CALLBACK(on_helpAbout_activate) },
  { "Quit", NULL, N_("_Quit"), "<control>Q", NULL, G_CALLBACK(on_fileQuit_activate) },
  { "Exit", NULL, N_("E_xit"), "<control>X", NULL, G_CALLBACK(on_winMain_delete_event) },
  { "Color", "hi_xo_colorselector", N_("_Color"), NULL, NULL, G_CALLBACK(on_colorButton_activate) }
};

static GtkActionEntry hildon_toggle_entries[] = {
  { "Fullscreen", "general_fullsize", N_("_Fullscreen"), "F11", NULL, G_CALLBACK(on_viewFullscreen_activate) },
  { "ApplyAllPages", NULL, N_("Apply _to all pages"), NULL, NULL, G_CALLBACK(on_journalApplyAllPages_activate) },
  { "Share", "general_share", N_("Share"), NULL, NULL, G_CALLBACK(hildon_share_tool) },
  { "Tools", NULL, N_("Tools"), NULL, NULL, G_CALLBACK(on_toolStackableWindow) },
  { "Settings", "general_settings", N_("Tools"), NULL, NULL, G_CALLBACK(on_toolStackableWindow) },
  { "Back", NULL, N_("Back"), NULL, NULL, G_CALLBACK(hildon_toolbar_switch) }
};

static GtkRadioActionEntry hildon_view_radio_entries[] = {
  { "viewContinuous", NULL, N_("_Continuous"), NULL, NULL, 0 },
  { "viewOnePage", NULL, N_("_One page"), NULL, NULL, 1}
};

static GtkRadioActionEntry hildon_paperColor_radio_entries[] = {
  { "papercolorWhite", NULL, N_("_white paper"), NULL, NULL, 0 },
  { "papercolorYellow", NULL, N_("_yellow paper"), NULL, NULL, 1 },
  { "papercolorPink", NULL, N_("_pink paper"), NULL, NULL, 2 },
  { "papercolorOrange", NULL, N_("_orange paper"), NULL, NULL, 3 },
  { "papercolorBlue", NULL, N_("_blue paper"), NULL, NULL, 4 },
  { "papercolorGreen", NULL, N_("_green paper"), NULL, NULL, 5 },
  { "papercolorNA", NULL, N_("NA"), NULL, NULL, 5 }
};

static GtkRadioActionEntry hildon_paperStyle_radio_entries[] = {
  { "paperstylePlain", NULL, N_("_plain"), NULL, NULL, 0 },
  { "paperstyleLined", NULL, N_("_lined"), NULL, NULL, 1 },
  { "paperstyleRuled", NULL, N_("_ruled"), NULL, NULL, 2 },
  { "paperstyleGraph", NULL, N_("_graph"), NULL, NULL, 3 },
  { "paperstyleNA", NULL, N_("NA"), NULL, NULL, 3 }
};

static GtkRadioActionEntry hildon_tools_radio_entries[] = {
  { "Pen", "hi_xo_pen", N_("_Pen"), "<control><shift>P", NULL, 0 },
  { "Eraser", "hi_xo_eraser", N_("_Eraser"), "<control><shift>E", NULL, 1 },
  { "Highlighter", "hi_xo_highlighter", N_("_Highlighter"), "<control><shift>H", NULL, 2 },
  { "Text", "hi_xo_text", N_("_Text"), "<control><shift>T", NULL, 3 },
  { "SelectRegion", NULL, N_("Select _rectangle"), "<control><shift>R", NULL, 4 },
  { "VerticalSpace", "hi_xo_verticalspace", N_("_Vertical space"), "<control><shift>V", NULL, 5 },
  { "Hand", "hi_xo_hand", N_("H_and tool"), "<control><shift>A", NULL, 6 },
  { "PenColor", "hi_xo_pencolor", N_("Pen _Color"), "<control><shift>C", NULL, 7 }
};

static GtkToggleActionEntry hildon_toolsOther_toggle_entries[] = {
  { "ShapeRecognizer", "hi_xo_ruler", N_("_Shape recognizer"), "<control><shift>S", NULL, G_CALLBACK(on_toolsReco_activate), 0 },
  { "Ruler", "hi_xo_ruler", N_("Ru_ler"), "<control><shift>L", NULL, G_CALLBACK(on_toolsRuler_activate), 0 }
};

static GtkRadioActionEntry hildon_penColor_radio_entries[] = {
  { "Black", "hi_xo_black", NULL, NULL, NULL, COLOR_BLACK },
  { "Blue", "hi_xo_blue", NULL, NULL, NULL, COLOR_BLUE },
  { "Red", "hi_xo_red", NULL, NULL, NULL, COLOR_RED },
  { "Green", "hi_xo_green", NULL, NULL, NULL, COLOR_GREEN },
  { "Yellow", "hi_xo_yellow", NULL, NULL, NULL, COLOR_YELLOW },
  { "Orange", "hi_xo_orange", NULL, NULL, NULL, COLOR_ORANGE },
  { "Magenta", "hi_xo_magenta", NULL, NULL, NULL, COLOR_MAGENTA }
};

static GtkRadioActionEntry hildon_penOptions_radio_entries[] = {
  { "PenThicknessVeryFine", "hi_xo_veryfine", N_("_very fine"), NULL, NULL, THICKNESS_VERYFINE },
  { "PenThicknessFine", "hi_xo_fine", N_("_fine"), NULL, NULL, THICKNESS_FINE },
  { "PenThicknessMedium", "hi_xo_medium", N_("_medium"), NULL, NULL, THICKNESS_MEDIUM },
  { "PenThicknessThick", "hi_xo_thick", N_("_thick"), NULL, NULL, THICKNESS_THICK },
  { "PenThicknessVeryThick", "hi_xo_verythick", N_("very t_hick"), NULL, NULL, THICKNESS_VERYTHICK }
};

static GtkRadioActionEntry hildon_eraserThicknessOptions_radio_entries[] = {
  { "EraserFine", NULL, N_("_fine"), NULL, NULL, 0 },
  { "EraserMedium", NULL, N_("_medium"), NULL, NULL, 1 },
  { "EraserThick", NULL, N_("_thick"), NULL, NULL, 2 }
};

static GtkRadioActionEntry hildon_eraserTypeOptions_radio_entries[] = {
  { "EraserStandard", NULL, N_("_standard"), NULL, NULL, 0 },
  { "EraserWhiteout", NULL, N_("_whiteout"), NULL, NULL, 1 },
  { "EraserDeleteStrokes", NULL, N_("_delete strokes"), NULL, NULL, 2 }
};

static GtkRadioActionEntry hildon_highlighterThicknessOptions_radio_entries[] = {
  { "HighlighterFine", NULL, N_("_fine"), NULL, NULL, 0 },
  { "HighlighterMedium", NULL, N_("_medium"), NULL, NULL, 1 },
  { "HighlighterThick", NULL, N_("_thick"), NULL, NULL, 2 }
};

/* Fremantle app menu */
typedef struct {
	gchar *title;
	GCallback callback;
	gint show;
	GtkWidget *widget;
} XournalHildonMenu;

static XournalHildonMenu fremantle_menu_filters[] = {
	{ "Notes", G_CALLBACK(hildon_note_switch), 1, NULL },
	{ "PDF", G_CALLBACK(hildon_pdf_switch), 1, NULL },
	{ "Drawing", G_CALLBACK(hildon_draw_switch), 1, NULL }
};

static XournalHildonMenu fremantle_menu_file_entries[] = {
	{ "New", G_CALLBACK(on_fileNew_activate), 1, NULL },
	{ "Open", G_CALLBACK(on_fileOpen_activate), 1, NULL },
	{ "Open background", G_CALLBACK(on_journalLoadBackground_activate), 1, NULL },
	{ "Save", G_CALLBACK(on_fileSave_activate), 1, NULL },
	{ "Save As", G_CALLBACK(on_fileSaveAs_activate), 1, NULL },
	{ "Export", G_CALLBACK(on_filePrintPDF_activate), 1, NULL },
	{ "Select layer", G_CALLBACK(on_comboLayer_changed), 1, NULL },
	{ "Delete layer", G_CALLBACK(on_journalDeleteLayer_activate), 1, NULL },
	{ "About", G_CALLBACK(hildon_about), 1, NULL }
};
#define HILDON_SHOW_LAYER_POS 6 // update this if you change position above

static XournalHildonMenu fremantle_menu_tools_entries[] = {
	{ "Pen...", G_CALLBACK(hildon_pen_activate), 0, NULL },
	{ "Eraser...", G_CALLBACK(hildon_eraser_activate), 0, NULL },
	{ "Highlighter...", G_CALLBACK(hildon_highlighter_activate), 0, NULL },
	{ "Zoom...", G_CALLBACK(hildon_zoom_activate), 0, NULL },
	{ "Layer...", G_CALLBACK(hildon_layer_activate), 0, NULL },
	{ "Page...", G_CALLBACK(hildon_page_activate), 0, NULL },
	{ "Paper...", G_CALLBACK(hildon_paper_activate), 0, NULL },
	{ "Recognizer", G_CALLBACK(on_toolsReco_activate), 0, NULL },
	{ "Select", G_CALLBACK(on_toolsSelectRectangle_activate), 0, NULL },
	{ "Vertical Space", G_CALLBACK(on_toolsVerticalSpace_activate), 0, NULL },
};

/* Icons */
static struct {
  gchar *filename;
  gchar *stock_id;
} hildon_stock_icons[] = {
  { PACKAGE_PIXMAP_DIR"/%s/draw-ink.png", "hi_xo_pressure" },
  { PACKAGE_PIXMAP_DIR"/%s/ruler.png", "hi_xo_ruler" },
  { PACKAGE_PIXMAP_DIR"/%s/highlighter.png", "hi_xo_highlighter" },
  { PACKAGE_PIXMAP_DIR"/%s/hand.png", "hi_xo_hand" },
  { PACKAGE_PIXMAP_DIR"/%s/pen.png", "hi_xo_pen" },
  { PACKAGE_PIXMAP_DIR"/%s/draw-ing.png", "hi_xo_pencolor" },
  { PACKAGE_PIXMAP_DIR"/%s/eraser.png", "hi_xo_eraser" },
  { PACKAGE_PIXMAP_DIR"/%s/color_selector.png", "hi_xo_colorselector" },
  { PACKAGE_PIXMAP_DIR"/%s/vertical_space.png", "hi_xo_verticalspace" },
  { PACKAGE_PIXMAP_DIR"/%s/draw-text.png", "hi_xo_text" },
  { PACKAGE_PIXMAP_DIR"/%s/prev.png", "hi_xo_prev" },
  { PACKAGE_PIXMAP_DIR"/%s/next.png", "hi_xo_next" },
  { PACKAGE_PIXMAP_DIR"/%s/newPageBefore.png", "hi_xo_newpagebefore" },
  { PACKAGE_PIXMAP_DIR"/%s/newPageAfter.png", "hi_xo_newpageafter" },
  { PACKAGE_PIXMAP_DIR"/%s/deletePage.png", "hi_xo_deletepage" },
  { PACKAGE_PIXMAP_DIR"/%s/veryFine.png", "hi_xo_veryfine" },
  { PACKAGE_PIXMAP_DIR"/%s/fine.png", "hi_xo_fine" },
  { PACKAGE_PIXMAP_DIR"/%s/medium.png", "hi_xo_medium" },
  { PACKAGE_PIXMAP_DIR"/%s/thick.png", "hi_xo_thick" },
  { PACKAGE_PIXMAP_DIR"/%s/veryThick.png", "hi_xo_verythick" },
  { PACKAGE_PIXMAP_DIR"/%s/black.png", "hi_xo_black" },
  { PACKAGE_PIXMAP_DIR"/%s/blue.png", "hi_xo_blue" },
  { PACKAGE_PIXMAP_DIR"/%s/red.png", "hi_xo_red" },
  { PACKAGE_PIXMAP_DIR"/%s/green.png", "hi_xo_green" },
  { PACKAGE_PIXMAP_DIR"/%s/yellow.png", "hi_xo_yellow" },
  { PACKAGE_PIXMAP_DIR"/%s/orange.png", "hi_xo_orange" },
  { PACKAGE_PIXMAP_DIR"/%s/magenta.png", "hi_xo_magenta" },
};
#endif
