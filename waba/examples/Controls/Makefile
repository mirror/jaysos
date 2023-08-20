# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4-p5 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ../..
prefix = /usr
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/waba
pkglibdir = $(libdir)/waba
pkgincludedir = $(includedir)/waba

top_builddir = ../..

ACLOCAL = aclocal -I macros_m4/misc -I macros_m4/java -I macros_m4/pilot -I macros_m4/toppers
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = i686-pc-linux-gnu
build_triplet = i686-pc-linux-gnu
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
target_alias = i686-pc-linux-gnu
target_triplet = i686-pc-linux-gnu
AR = ar
BEE_BIOS_CFLAGS = 
BUILD_PRC = build-prc
CC = gcc
CC_DEFINE = -DUSE_GLIB=1 -DLINUX=1 -DGUI_GTK=1 -DPLATFORM="\"Linux_GTK\"" 
CC_FOR_BUILD = gcc
CLASSPATH = /home/ja/wabacvs/waba/waba_classes
CPP = gcc -E
EXEGEN = exegen
FOR_PALM = 
FOR_WIN32 = 
GLIB_CFLAGS = 
GLIB_CONFIG = 
GLIB_LIBS = 
GTK_CFLAGS = -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include
GTK_CONFIG = /usr/bin/gtk-config
GTK_LIBS = -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
ITRON_CFLAGS = 
ITRON_CPU = 
ITRON_SYS = 
JAR = jar
JAR_CFLAGS = 
JAR_LDFLAGS = 
JAR_LIBS = 
JAVA = @JAVA@
JAVAC = javac
JAVACFLAGS = 
JAVAFLAGS = @JAVAFLAGS@
JAVAPREFIX = @JAVAPREFIX@
MAKEINFO = makeinfo
MINUS_RPM_PLATFORM = -i686-pc-linux-gnu
NM = nm
PACKAGE = waba
PGBOARD_CFLAGS = 
PGBOARD_LDFLAGS = 
PGBOARD_LIBS = 
PGUI_LIBS = 
PLATFORM = linux
RANLIB = ranlib
RM_CFLAGS = 
RM_LDFLAGS = 
RM_LIBS = 
RPM_CONFIGURE = ./configure  --with-javac=javac --prefix=/usr
RPM_PLATFORM = i686-pc-linux-gnu
RPM_PREFIX = /usr
RPM_REQUIRE_MDL = uczlib-i686-pc-linux-gnu waba-mdl = 1.5
RPM_ROOT = /usr
VERSION = 1.5
WABA_ID = 
WARP = warp
uudecode = @uudecode@
x_includes = NONE
x_libraries = NONE

classdir = ${datadir}/doc/waba/examples/Controls
EXTRA_DIST = Controls.java Controls.html clean.bat icon.bmp make.bat

class_DATA = Controls.class ControlPage.class ContainerPage.class
MOSTLYCLEANFILES = $(class_DATA)
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../vm/config.h
CONFIG_CLEAN_FILES = 
DATA =  $(class_DATA)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu examples/Controls/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


install-classDATA: $(class_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(classdir)
	@list='$(class_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(classdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(classdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(classdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(classdir)/$$p; \
	  fi; fi; \
	done

uninstall-classDATA:
	@$(NORMAL_UNINSTALL)
	list='$(class_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(classdir)/$$p; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = examples/Controls

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu examples/Controls/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am: install-classDATA
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-classDATA
uninstall: uninstall-am
all-am: Makefile $(DATA)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(classdir)


mostlyclean-generic:
	-test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: uninstall-classDATA install-classDATA tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Since there is only one source file for all the classes, make sure it
# only appears once in the list of files to compile
$(class_DATA): %.class: Controls.java
	olddir=`pwd` && cd `dirname $<` && javafile=`pwd`/`basename $<` && cd $${olddir} && echo $${javafile} >> $(top_builddir)/examples/to_recompile.txt
	touch $(class_DATA)

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
