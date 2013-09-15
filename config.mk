NAME = dwmstbar
VERSION = 1.2

# Customize below to fit your system

# paths
PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/include/X11
X11LIB = /usr/lib/X11

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} -I/usr/include/gnutls
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lasound -lgnutls

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
LDFLAGS = -g ${LIBS}

# compiler and linker
CC = cc
