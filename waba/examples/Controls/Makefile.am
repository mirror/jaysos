classdir = ${datadir}/doc/waba/examples/Controls

EXTRA_DIST = Controls.java
EXTRA_DIST += Controls.html clean.bat icon.bmp make.bat

class_DATA = Controls.class ControlPage.class ContainerPage.class
MOSTLYCLEANFILES = $(class_DATA)

# Since there is only one source file for all the classes, make sure it
# only appears once in the list of files to compile
$(class_DATA): %.class: Controls.java
	olddir=`pwd` && cd `dirname $<` && javafile=`pwd`/`basename $<` && cd $${olddir} && echo $${javafile} >> $(top_builddir)/examples/to_recompile.txt
	touch $(class_DATA)
