

#original Makefile.am contents follow:

#dataFiles = main.cpp cmakesimple.png CMakeLists.txt
#templateName = cmakesimple
#
#### no need to change below:
#template_DATA = $(templateName).kdevtemplate
#templatedir = ${appwizarddatadir}/templates
#
#appwizarddatadir = ${kde_datadir}/kdevappwizard
#$(templateName).tar.gz:
#	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
#	$(GZIP) -9 $(templateName).tar
#
#archivedir = ${appwizarddatadir}
#archive_DATA = $(templateName).tar.gz ${templateName}.png
#
#CLEANFILES = *.tar.gz

INCLUDE_DIRECTORIES(include)

AUX_SOURCE_DIRECTORY(src MAIN_SRCS)
AUX_SOURCE_DIRECTORY(src/client CLIENT_SRCS)
AUX_SOURCE_DIRECTORY(src/common COMMON_SRCS)
AUX_SOURCE_DIRECTORY(src/server SERVER_SRCS)

ADD_EXECUTABLE(openlierox ${MAIN_SRCS} ${CLIENT_SRCS} ${COMMON_SRCS} ${SERVER_SRCS})
