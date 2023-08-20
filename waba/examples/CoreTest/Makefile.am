classdir = ${datadir}/doc/waba/examples/CoreTest

EXTRA_DIST = CoreTest.java
EXTRA_DIST += CoreTest.html clean.bat make.bat icon.bmp test.bmp

class_DATA = CoreTest.class
MOSTLYCLEANFILES = $(class_DATA)

noinst_DATA = test.bmp

$(class_DATA): %.class: %.java
	olddir=`pwd` && cd `dirname $<` && javafile=`pwd`/`basename $<` && cd $${olddir} && echo $${javafile} >> $(top_builddir)/examples/to_recompile.txt
