INCLUDES = 	-DG_LOG_DOMAIN=\"hildon-extras-@API_VERSION_MAJOR@\" \
			-DLOCALEDIR=\"$(localedir)\"

lib_LTLIBRARIES = \
		libhildon-extras-@API_VERSION_MAJOR@.la

libhildon_extras_@API_VERSION_MAJOR@_la_LDFLAGS = \
		$(HILDON_WIDGETS_LT_LDFLAGS)

libhildon_extras_@API_VERSION_MAJOR@_la_LIBADD = \
		$(HILDON_LIBS) \
		$(MCE_LIBS) \
		$(DBUS_LIBS) \
		$(LIBOSSO_LIBS)

libhildon_extras_@API_VERSION_MAJOR@_la_CFLAGS	= \
		$(HILDON_CFLAGS) \
		$(MCE_CFLAGS) \
		$(DBUS_CFLAGS) \
		$(LIBOSSO_CFLAGS)

libhildon_extras_@API_VERSION_MAJOR@_la_SOURCES = \
                he-about-dialog.c \
		he-check-button.c \
		he-color-selector.c \
		he-color-dialog.c \
		he-color-button.c \
		he-font-selector.c \
		he-font-dialog.c \
		he-font-button.c \
		he-fullscreen-button.c \
		he-helper.c \
		he-simple-color-dialog.c \
		he-arrowed-entry.c

libhildon_extras_@API_VERSION_MAJOR@_includedir = \
		$(includedir)/$(PACKAGE)-$(API_VERSION_MAJOR)/$(PACKAGE)
	
libhildon_extras_@API_VERSION_MAJOR@_public_headers = \
                he-about-dialog.h \
		he-check-button.h \
		he-color-selector.h \
		he-color-dialog.h \
		he-color-button.h \
		he-font-selector.h \
		he-font-dialog.h \
		he-font-button.h \
		he-fullscreen-button.h \
		he-helper.h \
		he-simple-color-dialog.h \
		he-arrowed-entry.h

libhildon_extras_@API_VERSION_MAJOR@_include_HEADERS = \
		$(libhildon_extras_@API_VERSION_MAJOR@_public_headers)
