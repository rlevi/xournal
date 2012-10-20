#!/bin/sh

set -x
glib-gettextize --copy --force
libtoolize --automake
intltoolize --automake --copy --force
aclocal
autoconf
autoheader
automake --add-missing --foreign
./configure --prefix=/usr
