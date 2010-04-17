#include <gtk/gtk.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <hildon/hildon.h>
#include <hildon/hildon-defines.h>
#include <hildon/hildon-window.h>

#include <libosso.h>

#include "xo-callbacks.h"

#define HILDON_NUM_TOOLBARS		6

#define HILDON_TOOLBAR_NOTE_LANDSCAPE	0
#define HILDON_TOOLBAR_NOTE_PORTRAIT	1
#define HILDON_TOOLBAR_PDF_LANDSCAPE	2
#define HILDON_TOOLBAR_PDF_PORTRAIT	3
#define HILDON_TOOLBAR_DRAW_LANDSCAPE	4
#define HILDON_TOOLBAR_DRAW_PORTRAIT	5

extern osso_context_t *ctx;

GtkWidget *hildon_toolbars[HILDON_NUM_TOOLBARS];
GtkWidget *hildon_cur_toolbar_portrait;
GtkWidget *hildon_cur_toolbar_landscape;

HildonColorButton *buttonColor;
GtkWidget *pickerButton;

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
  { "Undo", GTK_STOCK_UNDO, N_("_Undo"), "<control>Z", NULL, G_CALLBACK(on_editUndo_activate) },
  { "Redo", GTK_STOCK_REDO, N_("_Redo"), "<control>Y", NULL, G_CALLBACK(on_editRedo_activate) },
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
  { "PreviousPage", GTK_STOCK_GO_BACK, N_("_Previous page"), NULL, NULL, G_CALLBACK(on_viewPreviousPage_activate) },
  { "NextPage", GTK_STOCK_GO_FORWARD, N_("_Next page"), NULL, NULL, G_CALLBACK(on_viewNextPage_activate) },
  { "LastPage", GTK_STOCK_GOTO_LAST, N_("_Last page"), NULL, NULL, G_CALLBACK(on_viewLastPage_activate) },
  { "ShowLayers", GTK_STOCK_REMOVE, N_("Select _layer"), NULL, NULL, G_CALLBACK(on_HildonShowLayer_activate) },
  { "ShowLayer", GTK_STOCK_ADD, N_("_Show layer"), NULL, NULL, G_CALLBACK(on_viewShowLayer_activate) },
  { "HideLayer", GTK_STOCK_REMOVE, N_("_Hide layer"), NULL, NULL, G_CALLBACK(on_viewHideLayer_activate) },
  { "NewPageBefore", NULL, N_("New page _before"), NULL, NULL, G_CALLBACK(on_journalNewPageBefore_activate) },
  { "NewPageAfter", NULL, N_("New page _after"), NULL, NULL, G_CALLBACK(on_journalNewPageAfter_activate) },
  { "NewPageEnd", NULL, N_("New page at _end"), NULL, NULL, G_CALLBACK(on_journalNewPageEnd_activate) },
  { "DeletePage", NULL, N_("_Delete page"), NULL, NULL, G_CALLBACK(on_journalDeletePage_activate) },
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
  { "Pressure", "pressure", N_("_Pressure sensitivity"), NULL, NULL, G_CALLBACK(on_optionsPressureSensitive_activate) },
  { "ProgressiveBG",NULL, N_("_Progressive backgrounds"), NULL, NULL, G_CALLBACK(on_optionsProgressiveBG_activate) },
  { "PrintRuling", NULL, N_("_Print paper _ruling"), NULL, NULL, G_CALLBACK(on_optionsPrintRuling_activate) },
  { "LeftHanded", NULL, N_("Left-Handed scrollbar"), NULL, NULL, G_CALLBACK(on_optionsLeftHanded_activate) },
  { "AutoSavePrefs", NULL, N_("A_uto-save preferences"), NULL, NULL, G_CALLBACK(on_optionsAutoSavePrefs_activate) },
  { "SavePreferences", NULL, N_("_Save preferences"), NULL, NULL, G_CALLBACK(on_optionsSavePreferences_activate) },
  { "Help", NULL, N_("_Help..."), NULL, NULL, G_CALLBACK(on_HildonHelp_activate) },
  { "About", NULL, N_("_About..."), NULL, NULL, G_CALLBACK(on_helpAbout_activate) },
  { "Quit", NULL, N_("_Quit"), "<control>Q", NULL, G_CALLBACK(on_fileQuit_activate) },
  { "Exit", NULL, N_("E_xit"), "<control>X", NULL, G_CALLBACK(on_winMain_delete_event) },
  { "Color", "color_selector", N_("_Color"), NULL, NULL, G_CALLBACK(on_colorButton_activate) }
};

static GtkActionEntry hildon_toggle_entries[] = {
  { "Fullscreen", "general_fullsize", N_("_Fullscreen"), "F11", NULL, G_CALLBACK(on_viewFullscreen_activate) },
  { "ApplyAllPages", NULL, N_("Apply _to all pages"), NULL, NULL, G_CALLBACK(on_journalApplyAllPages_activate) },
  { "Share", "general_share", N_("Share"), NULL, NULL, G_CALLBACK(hildon_share_tool) },
  { "Tools", NULL, N_("Tools"), NULL, NULL, G_CALLBACK(on_toolStackableWindow) },
  { "Settings", "general_settings", N_("Tools"), NULL, NULL, G_CALLBACK(on_toolStackableWindow) }
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
  { "Pen", "pen", N_("_Pen"), "<control><shift>P", NULL, 0 },
  { "Eraser", "eraser", N_("_Eraser"), "<control><shift>E", NULL, 1 },
  { "Highlighter", "highlighter", N_("_Highlighter"), "<control><shift>H", NULL, 2 },
  { "Text", "text", N_("_Text"), "<control><shift>T", NULL, 3 },
  { "SelectRegion", NULL, N_("Select _rectangle"), "<control><shift>R", NULL, 4 },
  { "VerticalSpace", "vertical_space", N_("_Vertical space"), "<control><shift>V", NULL, 5 },
  { "Hand", "hand", N_("H_and tool"), "<control><shift>A", NULL, 6 }
};

static GtkToggleActionEntry hildon_toolsOther_toggle_entries[] = {
  { "ShapeRecognizer", "ruler", N_("_Shape recognizer"), "<control><shift>S", NULL, G_CALLBACK(on_toolsReco_activate), 0 },
  { "Ruler", "ruler", N_("Ru_ler"), "<control><shift>L", NULL, G_CALLBACK(on_toolsRuler_activate), 0 }
};

static GtkRadioActionEntry hildon_penOptions_radio_entries[] = {
  { "penThicknessVeryFine", NULL, N_("_very fine"), NULL, NULL, 0 },
  { "penThicknessFine", NULL, N_("_fine"), NULL, NULL, 1 },
  { "penThicknessMedium", NULL, N_("_medium"), NULL, NULL, 2 },
  { "penThicknessThick", NULL, N_("_thick"), NULL, NULL, 3 },
  { "penThicknessVeryThick", NULL, N_("very t_hick"), NULL, NULL, 4 }
};

static GtkRadioActionEntry hildon_eraserThicknessOptions_radio_entries[] = {
  { "eraserFine", NULL, N_("_fine"), NULL, NULL, 0 },
  { "eraserMedium", NULL, N_("_medium"), NULL, NULL, 1 },
  { "eraserThick", NULL, N_("_thick"), NULL, NULL, 2 }
};

static GtkRadioActionEntry hildon_eraserTypeOptions_radio_entries[] = {
  { "eraserStandard", NULL, N_("_standard"), NULL, NULL, 0 },
  { "eraserWhiteout", NULL, N_("_whiteout"), NULL, NULL, 1 },
  { "eraserDeleteStrokes", NULL, N_("_delete strokes"), NULL, NULL, 2 }
};

static GtkRadioActionEntry hildon_highlighterThicknessOptions_radio_entries[] = {
  { "highlighterFine", NULL, N_("_fine"), NULL, NULL, 0 },
  { "highlighterMedium", NULL, N_("_medium"), NULL, NULL, 1 },
  { "highlighterThick", NULL, N_("_thick"), NULL, NULL, 2 }
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
	{ "Save", G_CALLBACK(on_fileSave_activate), 1, NULL },
	{ "Save As", G_CALLBACK(on_fileSaveAs_activate), 1, NULL },
	{ "Export", G_CALLBACK(on_filePrintPDF_activate), 1, NULL },
	{ "Share", G_CALLBACK(hildon_share_tool), 1, NULL },
	{ "Show layer", G_CALLBACK(on_comboLayer_changed), 1, NULL },
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
  { PACKAGE_PIXMAP_DIR"/%s/draw-ink.png", "pressure" },
  { PACKAGE_PIXMAP_DIR"/%s/ruler.png", "ruler" },
  { PACKAGE_PIXMAP_DIR"/%s/highlighter.png", "highlighter" },
  { PACKAGE_PIXMAP_DIR"/%s/hand.png", "hand" },
  { PACKAGE_PIXMAP_DIR"/%s/pen.png", "pen" },
  { PACKAGE_PIXMAP_DIR"/%s/eraser.png", "eraser" },
  { PACKAGE_PIXMAP_DIR"/%s/color_selector.png", "color_selector" },
  { PACKAGE_PIXMAP_DIR"/%s/vertical_space.png", "vertical_space" },
  { PACKAGE_PIXMAP_DIR"/%s/draw-text.png", "text" },
};
