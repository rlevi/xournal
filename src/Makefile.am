## Process this file with automake to produce Makefile.in

SUBDIRS = ttsubset

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_PIXMAP_DIR=\""$(pixmapdir)"\" \
	-DPACKAGE_UIXML_DIR=\""$(uixmldir)"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = xournal

xournal_SOURCES = \
	main.c xournal.h \
	xo-misc.c xo-misc.h \
	xo-file.c xo-file.h \
	xo-paint.c xo-paint.h \
	xo-print.c xo-print.h \
	xo-support.c xo-support.h \
	xo-hildon_ui.c xo-hildon_ui.h \
	xo-callbacks.c xo-callbacks.h \
	xo-shapes.c xo-shapes.h \
	hildon-extras/he-about-dialog.c hildon-extras/he-about-dialog.h \
	hildon-extras/he-fullscreen-button.c hildon-extras/he-fullscreen-button.h \
	hildon-extras/he-color-selector.c hildon-extras/he-color-selector.h \
	hildon-extras/he-color-dialog.c hildon-extras/he-color-dialog.h \
	hildon-extras/he-simple-color-dialog.c hildon-extras/he-simple-color-dialog.h \
	hildon-extras/he-color-button.c hildon-extras/he-color-button.h \
	hildon-extras/he-color-size-selector.c hildon-extras/he-color-size-selector.h \
	hildon-extras/he-color-size-dialog.c hildon-extras/he-color-size-dialog.h

xournal_LDADD = ttsubset/libttsubset.a @PACKAGE_LIBS@ $(INTLLIBS)
