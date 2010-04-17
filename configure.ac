#
# Xournal port for Maemo
#
# Port author homepage: http://anidel.blogpost.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
#

AC_INIT(xournal, 0.4.5)
AC_PREREQ(2.59)

AM_INIT_AUTOMAKE([1.7.9])
AC_CONFIG_HEADERS([config.h])

AC_GNU_SOURCE
AC_CANONICAL_HOST
AM_MAINTAINER_MODE

AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INTLTOOL([0.23])
AC_PROG_RANLIB

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 src/ttsubset/Makefile
		 data/Makefile
		 po/Makefile.in
		 po/Makefile])

hildon_dependencies="gtk+-2.0 >= 2.4.0 libgnomecanvas-2.0 >= 2.4.0 poppler-glib hildon-1 hildon-fm-2 libosso dbus-1 libmodest-dbus-client-1.0 >= 1.0"

PKG_CHECK_MODULES(PACKAGE, [$hildon_dependencies], USE_HILDON=yes, USE_HILDON=no)
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)
AC_DEFINE(USE_HILDON, 1, [build with hildon libs])
AC_SUBST(USE_HILDON)

# Locale
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "${GETTEXT_PACKAGE}", [Name of gettext package])
ALL_LINGUAS="en_GB it_IT"
AM_GLIB_GNU_GETTEXT

localedir=`$PKG_CONFIG osso-af-settings --variable=localedir`

# To make application visible in maemo Task Navigator it needs a Desktop
# file for the application.
# D-BUS service file is needed to be able to launch the maemo application
# and connect it to D-BUS services.
# The following line defines install directories for these files.
desktopentrydir=`$PKG_CONFIG osso-af-settings --variable=desktopentrydir`
serviceentrydir=`$PKG_CONFIG osso-af-settings --variable=dbusservicedir`

# Application pixmaps install directory
pixmapdir_base=`$PKG_CONFIG osso-af-settings --variable=hildonpixmapdir`
pixmapdir=$pixmapdir_base/xournal

# Default theme
themedefaultdir=$pixmapdir_base/xournal/default

# Application icon install directories
icon_26x26dir=$datadir/icons/hicolor/26x26/hildon
icon_34x34dir=$datadir/icons/hicolor/34x34/hildon
icon_40x40dir=$datadir/icons/hicolor/40x40/hildon
icon_48x48dir=$datadir/icons/hicolor/48x48/hildon
icon_scalabledir=$datadir/icons/hicolor/scalable/hildon

# application/x-xoj mime icons
icon_mimetype24dir=$datadir/icons/hicolor/24x24/mimetypes
icon_mimetype48dir=$datadir/icons/hicolor/48x48/mimetypes
icon_mimetype64dir=$datadir/icons/hicolor/64x64/mimetypes


# Help file install directories
helpfile_engbdir=$datadir/osso-help/en_GB
helpfile_enusdir=$datadir/osso-help/en_US
helpfile_rurudir=$datadir/osso-help/ru_RU
helpgraphicsdir=$datadir/osso-help/graphics

# Release notes install directory
newsdir=$datadir/xournal

# config file install directory
dotconfigdir=$datadir/xournal

# UI xml file
uixmldir=$datadir/xournal

# the MIME type binding directory
mimedir=$datadir/mime/packages

# backup/restore directory
backupdir=/etc/osso-backup/applications

# Define as variables in Makefiles
AC_SUBST(localedir)
AC_SUBST(uixmldir)
AC_SUBST(themedefaultdir)
AC_SUBST(desktopentrydir)
AC_SUBST(serviceentrydir)
AC_SUBST(pixmapdir)
AC_SUBST(icon_26x26dir)
AC_SUBST(icon_34x34dir)
AC_SUBST(icon_40x40dir)
AC_SUBST(icon_48x48dir)
AC_SUBST(icon_scalabledir)
AC_SUBST(icon_mimetype24dir)
AC_SUBST(icon_mimetype48dir)
AC_SUBST(icon_mimetype64dir)
AC_SUBST(helpfile_engbdir)
AC_SUBST(helpfile_enusdir)
AC_SUBST(helpfile_rurudir)
AC_SUBST(newsdir)
AC_SUBST(helpgraphicsdir)
AC_SUBST(dotconfigdir)
AC_SUBST(mimedir)
AC_SUBST(backupdir)

AC_DEFINE_UNQUOTED([LOCALEDIR], "${localedir}", [Runtime locale catalog files path])
AC_DEFINE_UNQUOTED([PIXMAPDIR], "${pixmapdir}", [Runtime pixmap files path])
AC_DEFINE_UNQUOTED([UIXMLDIR],  "${uixmldir}",  [UI xml definition path])

AC_OUTPUT
