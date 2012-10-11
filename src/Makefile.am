INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @APP_CFLAGS@ @EXPAT_CFLAGS@ @X_CFLAGS@ -Wall
bin_PROGRAMS = xoo

xoo_SOURCES =  main.c button.c callbacks.h callbacks.c config.c keys.c fakedev.h prefs.c prefs.h

xoo_LDADD = @APP_LIBS@ @EXPAT_LIBS@ @X_LIBS@ @XTEST_LIBS@
