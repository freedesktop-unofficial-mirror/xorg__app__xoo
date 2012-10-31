devicedir = $(pkgdatadir)
dist_device_DATA = dot.png generic_btn_down.png \
	generic.xml generic.png \
	ipaq3800.xml ipaq3800.png \
	ipaq4700.xml ipaq4700.png \
	nokia-n800.xml nokia-n800.png

uidir = $(pkgdatadir)
dist_ui_DATA = xoo.ui

desktopdir = $(datadir)/applications
dist_desktop_DATA = xoo.desktop

pixmapdir = $(datadir)/pixmaps
dist_pixmap_DATA = xoo.png

convertdir = $(datadir)/GConf/gsettings
dist_convert_DATA = xoo.convert

gsettings_SCHEMAS = org.x.Xoo.gschema.xml

@GSETTINGS_RULES@

EXTRA_DIST = $(gsettings_SCHEMAS)
