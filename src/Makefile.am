INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @APP_CFLAGS@ @EXPAT_CFLAGS@ @X_CFLAGS@ -Wall
bin_PROGRAMS = xoo

if HAVE_GCONF
GCONF_SOURCES = prefs.c prefs.h
endif
xoo_SOURCES =  main.c button.c callbacks.h callbacks.c config.c keys.c fakedev.h $(GCONF_SOURCES)

xoo_LDADD = @APP_LIBS@ @EXPAT_LIBS@ @X_LIBS@ @XTEST_LIBS@
