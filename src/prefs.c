/* Xoo - a graphical wrapper around xnest
 *
 *  Copyright 2004,2005 Matthew Allum, Openedhand Ltd <mallum@o-hand.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "config.h"

#include <string.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include "prefs.h"

gboolean fakeapp_restart_server (FakeApp * app);  /* TODO: move to a header */

static GSettings *gsettings = NULL;

void
gsettings_prefs_init (FakeApp * app)
{
  char *s;
  gsettings = g_settings_new ("org.x.Xoo");
  g_return_if_fail (gsettings != NULL);

  s = g_settings_get_string (gsettings, "display");
  if (s != NULL && *s != '\0')
    {
      app->xnest_dpy_name = s;
    }

  s = g_settings_get_string (gsettings, "xserver");
  if (s != NULL && *s != '\0')
    {
      app->xnest_bin_path = s;
    }

  s = g_settings_get_string (gsettings, "xserver-options");
  if (s != NULL && *s != '\0')
    {
      app->xnest_bin_options = s;
    }

  s = g_settings_get_string (gsettings, "startup-command");
  if (s != NULL && *s != '\0')
    {
      app->start_cmd = s;
    }

}

void
on_preferences_activate (GtkMenuItem * menuitem, FakeApp * app)
{
  gtk_entry_set_text (GTK_ENTRY (app->entry_display), app->xnest_dpy_name);
  gtk_entry_set_text (GTK_ENTRY (app->entry_server), app->xnest_bin_path);
  gtk_entry_set_text (GTK_ENTRY (app->entry_options),
		      app->xnest_bin_options ? app->xnest_bin_options : "");

  gtk_entry_set_text (GTK_ENTRY (app->entry_start),
		      app->start_cmd ? app->start_cmd : "");

  gtk_window_present (GTK_WINDOW (app->prefs_window));
}

void
on_prefs_apply_clicked (GtkWidget * widget, FakeApp * app)
{
  const char *s = NULL;

  s = gtk_entry_get_text (GTK_ENTRY (app->entry_display));
  g_settings_set_string (gsettings, "display", s);

  s = gtk_entry_get_text (GTK_ENTRY (app->entry_server));
  g_settings_set_string (gsettings, "xserver", s);

  s = gtk_entry_get_text (GTK_ENTRY (app->entry_options));
  g_settings_set_string (gsettings, "xserver-options", s);

  s = gtk_entry_get_text (GTK_ENTRY (app->entry_start));
  g_settings_set_string (gsettings, "startup-command", s);

  gsettings_prefs_init (app);
  fakeapp_restart_server (app);

  gtk_widget_hide (app->prefs_window);
}

void
on_prefs_cancel_clicked (GtkWidget * widget, FakeApp * app)
{
  gtk_widget_hide (app->prefs_window);
}
