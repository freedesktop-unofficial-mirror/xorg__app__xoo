#include <gtk/gtkmenuitem.h>
#include "fakedev.h"

#define GCONF_DISPLAY "/apps/matchbox-nest/display"
#define GCONF_SERVER "/apps/matchbox-nest/xserver"
#define GCONF_SERVER_OPTIONS "/apps/matchbox-nest/xserver-options"

void gconf_prefs_init(FakeApp *app);

void on_preferences_activate (GtkMenuItem *menuitem, FakeApp *app);
void on_prefs_apply_clicked (GtkWidget *widget, FakeApp *app);
void on_prefs_cancel_clicked (GtkWidget *widget, FakeApp *app);
