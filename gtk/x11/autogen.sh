#!/bin/sh

set -x
libtoolize --automake
aclocal
autoconf
autoheader
automake --add-missing --foreign
