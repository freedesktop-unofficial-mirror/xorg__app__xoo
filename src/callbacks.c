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

#include <signal.h>
#include <gtk/gtk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkwindow.h>
#include "fakedev.h"

extern pid_t xnest_pid;

void
on_send_signal_activate (GtkMenuItem * menuitem, FakeApp * app)
{
  g_return_if_fail (app->xnest_pid != 0);
  kill (app->xnest_pid, SIGUSR1);
}

static void
quit (FakeApp * app)
{
  kill (app->xnest_pid, SIGKILL);
  gtk_main_quit ();
}

void
on_quit_activate (GtkMenuItem * menuitem, FakeApp * app)
{
  quit (app);
}

void
on_window_destroy (GtkObject * object, FakeApp * app)
{
  quit (app);
}

gboolean
on_popup_menu_show (GtkWidget * widget, GdkEventButton * event, FakeApp * app)
{
  if (event->button == 3)
    {
      gtk_menu_popup (GTK_MENU (app->popupmenu), NULL, NULL, NULL, NULL,
		      event->button, event->time);
      return TRUE;
    }
  return FALSE;
}

void
on_show_decorations_toggle (GtkCheckMenuItem * menuitem, FakeApp * app)
{
  if (gtk_window_get_decorated (GTK_WINDOW (app->window)))
    {
      GdkBitmap *mask;
      mask =
	gdk_pixmap_new (app->fixed->window, app->device_width,
			app->device_height, 1);
      gdk_pixbuf_render_threshold_alpha (app->device_img, mask, 0, 0, 0, 0,
					 -1, -1, 128);
      gtk_widget_realize (app->window);
      gdk_window_shape_combine_mask (app->window->window, mask, 0, 0);

      gtk_widget_hide (app->menubar);
      gtk_window_set_decorated (GTK_WINDOW (app->window), FALSE);
      gtk_check_menu_item_set_active (menuitem, FALSE);
    }
  else
    {
      gdk_window_shape_combine_mask (app->window->window, NULL, 0, 0);
      gtk_widget_show (app->menubar);
      gtk_window_set_decorated (GTK_WINDOW (app->window), TRUE);
      gtk_check_menu_item_set_active (menuitem, TRUE);
    }
}

void
on_about_activate (GtkMenuItem * menuitem, FakeApp * app)
{
  gtk_window_present (GTK_WINDOW (app->about_window));
}

gboolean
on_delete_event_hide (GtkWidget * widget, GdkEvent * event, FakeApp * app)
{
  gtk_widget_hide (widget);
  return TRUE;
}

void
on_select_device (GtkMenuItem * menuitem, FakeApp * app)
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("Open Device",
					GTK_WINDOW (app->window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;


      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      /****** DONT TRY THIS AT HOME KIDS. *******/

      /* xxx FIXME, This is gross - just a very nasty hack for now xxx */

      kill (xnest_pid, 9);
      sleep (2);
      execl ("/bin/sh", "sh", "-c", "xoo", "--device", filename, NULL);

      g_warning ("Failed load device %s\n", filename);

      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}
