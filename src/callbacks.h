#include <gtk/gtkmenuitem.h>

/* GTK+ callbacks */

void 
on_send_signal_activate (GtkWidget *menuitem, FakeApp *app);

void 
on_preferences_activate (GtkMenuItem *menuitem, FakeApp *app);

void 
on_quit_activate (GtkMenuItem *menuitem, FakeApp *app);

void 
on_about_activate (GtkMenuItem *menuitem, FakeApp *app);

void 
on_window_destroy (GtkObject *widget, FakeApp *app);

gboolean 
on_popup_menu_show (GtkWidget *widget, GdkEventButton *event, FakeApp *app);

void 
on_show_decorations_toggle (GtkMenuItem *menuitem, FakeApp *app);

gboolean 
on_delete_event_hide (GtkWidget *widget, GdkEvent *event, FakeApp *app);

void 
on_select_device (GtkMenuItem *menuitem, FakeApp *app);
