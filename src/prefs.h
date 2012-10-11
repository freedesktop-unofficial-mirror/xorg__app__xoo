#include <gtk/gtk.h>
#include "fakedev.h"

void 
gsettings_prefs_init (FakeApp * app);

void 
on_preferences_activate (GtkMenuItem * menuitem, FakeApp * app);

void 
on_prefs_apply_clicked (GtkWidget * widget, FakeApp * app);

void 
on_prefs_cancel_clicked (GtkWidget * widget, FakeApp * app);
