#include <gtk/gtkmenuitem.h>
#include "fakedev.h"

#define GCONF_DISPLAY "/apps/Xoo/display"
#define GCONF_SERVER "/apps/Xoo/xserver"
#define GCONF_SERVER_OPTIONS "/apps/Xoo/xserver-options"
#define GCONF_START_CMD "/apps/Xoo/startup-command"

void gconf_prefs_init(FakeApp *app);

void on_preferences_activate (GtkMenuItem *menuitem, FakeApp *app);
void on_prefs_apply_clicked (GtkWidget *widget, FakeApp *app);
void on_prefs_cancel_clicked (GtkWidget *widget, FakeApp *app);
