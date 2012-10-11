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
on_window_destroy (GInitiallyUnowned * object, FakeApp * app)
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
  GdkWindow *gdk_window;

  gdk_window = gtk_widget_get_window (app->window);
  if (gtk_window_get_decorated (GTK_WINDOW (app->window)))
    {
	  /* we're shaping the window only if device image has an alpha channel */
      if (gdk_pixbuf_get_has_alpha (app->device_img))
        {
          gint root_x, root_y;
          cairo_surface_t *mask;
          cairo_region_t *region;
          cairo_t *cr;

          mask = cairo_image_surface_create (CAIRO_FORMAT_A1,
		 app->device_width, app->device_height);

          cr = cairo_create (mask);
          gdk_cairo_set_source_pixbuf (cr, app->device_img, 0, 0);
          cairo_paint (cr);

          region = gdk_cairo_region_create_from_surface (mask);

          gtk_window_get_position (GTK_WINDOW (app->window), &root_x, &root_y);
          gtk_widget_hide (app->window);
          gdk_window_shape_combine_region (gdk_window, region, 0, 0);
          gtk_widget_shape_combine_region (app->window, region);
          gtk_widget_show (app->window);

          cairo_region_destroy (region);
          cairo_destroy (cr);
          cairo_surface_destroy (mask);

          gtk_widget_realize (app->window);
          gtk_window_move (GTK_WINDOW (app->window), root_x, root_y);

          gtk_widget_hide (app->menubar);
          gtk_window_set_decorated (GTK_WINDOW (app->window), FALSE);
          gtk_check_menu_item_set_active (menuitem, FALSE);
        }
      }
  else
    {
      gtk_widget_shape_combine_region (app->window, NULL);
      gdk_window_shape_combine_region (gdk_window, NULL, 0, 0);

      gtk_widget_show (app->menubar);
      gtk_window_set_decorated (GTK_WINDOW (app->window), TRUE);
      gtk_check_menu_item_set_active (menuitem, TRUE);
    }
}

void
on_about_activate (GtkMenuItem * menuitem, FakeApp * app)
{
  gtk_show_about_dialog (NULL,
                         "program-name", "Xoo",
                         "version", VERSION,
                         "copyright", "Copyright © 2004 OpenedHand Ltd\ninfo@o-hand.com",
                         "comments", "A tool for simulating X-based small-screen devices",
                         "logo-icon-name", "xoo",
                         "license-type", GTK_LICENSE_GPL_2_0,
                         NULL);
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
  GtkFileFilter *filter;

  filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.xml");

  dialog = gtk_file_chooser_dialog_new ("Open Device",
					GTK_WINDOW (app->window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), PKGDATADIR);
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char cmd[PATH_MAX + 13];
      char *filename;


      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      /****** DONT TRY THIS AT HOME KIDS. *******/

      /* xxx FIXME, This is gross - just a very nasty hack for now xxx */

      kill (xnest_pid, SIGKILL);
      sleep (2);
      sprintf (cmd, "xoo --device %s", filename);
      execl ("/bin/sh", "sh", "-c", cmd, NULL);

      g_warning ("Failed load device %s\n", filename);

      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}
