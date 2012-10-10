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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "fakedev.h"
#include "callbacks.h"
#include "prefs.h"

#include <glib/gi18n.h>

#define XNEST_BIN "/usr/bin/Xephyr"
#define XNEST_DPY ":1"

/* Used by the signal handler to detect which child has died */
pid_t xnest_pid;

static gboolean key_event (GtkWidget * widget, GdkEventKey * event,
			   FakeApp * app);

static void fakeapp_catch_sigchild (int sign);

static char *
find_spare_dpy (void)
{
  int i;
  char buf[100];   
  
  for (i = 0; i < 256; i++) {
    g_snprintf (buf, sizeof(buf), "/tmp/.X%d-lock", i);
    if (access (buf, F_OK) != 0) {
      return g_strdup_printf (":%i", i);
    }
  }
  return NULL;
}

FakeApp *
fakeapp_new (void)
{
  GtkBuilder *builder;
  GtkWidget  *widget;
  FakeApp *app = g_new0 (FakeApp, 1);

  app->xnest_dpy_name = find_spare_dpy ();
  app->xnest_bin_path = XNEST_BIN;
  app->xnest_bin_options = strdup ("-ac");	/* freed if changed */

  app->key_rep_init_timeout.tv_usec = 0;
  app->key_rep_init_timeout.tv_sec = 1;

  app->key_rep_timeout.tv_usec = 40000;
  app->key_rep_timeout.tv_sec = 0;

  app->win_title = "Xoo";

  GError *error = NULL;
  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_file (builder, PKGDATADIR "/data/xoo.ui", &error))
    {
	g_warning ("Coulnd't load builder ui file : %s", error->message);
	g_error_free (error);
    }

  g_assert (builder != NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "send_signal"));
  g_object_connect (G_OBJECT (widget), "activate",
    G_CALLBACK (on_send_signal_activate), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "quit"));
  g_object_connect (G_OBJECT (widget), "activate",
    G_CALLBACK (on_quit_activate), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "about"));
  g_object_connect (G_OBJECT (widget), "activate",
    G_CALLBACK (on_about_activate), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
  g_object_connect (G_OBJECT (widget), "destroy",
    G_CALLBACK (on_window_destroy), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "eventbox1"));
  g_object_connect (G_OBJECT (widget), "button_press_event",
    G_CALLBACK (on_popup_menu_show), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "show_decorations"));
  g_object_connect (G_OBJECT (widget), "activate",
    G_CALLBACK (on_show_decorations_toggle), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "aboutwindow"));
  g_object_connect (G_OBJECT (widget), "delete_event",
    G_CALLBACK (on_delete_event_hide), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "prefswindow"));
  g_object_connect (G_OBJECT (widget), "delete_event",
    G_CALLBACK (on_delete_event_hide), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "aboutwindow"));
  g_object_connect (G_OBJECT (widget), "delete_event",
    G_CALLBACK (on_delete_event_hide), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "select_device"));
  g_object_connect (G_OBJECT (widget), "activate",
    G_CALLBACK (on_select_device), app, NULL);

#if HAVE_GCONF
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "preferences"));
  g_object_connect (G_OBJECT (widget), "activate",
    G_CALLBACK (on_preferences_activate), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "button_apply"));
  g_object_connect (G_OBJECT (widget), "clicked",
    G_CALLBACK (on_prefs_apply_clicked), app, NULL);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "button_close"));
  g_object_connect (G_OBJECT (widget), "clicked",
    G_CALLBACK (on_prefs_cancel_clicked), app, NULL);
#else
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "preferences"));
  gtk_widget_hide (widget);

#endif
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
  app->window = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "menubar"));
  app->menubar = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "fixed"));
  app->fixed = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "back"));
  app->back = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "winnest"));
  app->winnest = widget;

  g_signal_connect (app->window, "key-press-event",
		    (GCallback) key_event, app);

  g_signal_connect (app->window, "key-release-event",
		    (GCallback) key_event, app);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "prefswindow"));
  app->prefs_window = widget;
  gtk_window_set_transient_for (GTK_WINDOW (app->prefs_window),
				GTK_WINDOW (app->window));

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "entry_display"));
  app->entry_display = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "entry_server"));
  app->entry_server = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "entry_options"));
  app->entry_options = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "entry_start"));
  app->entry_start = widget;

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "send_signal"));
  app->debug_menu = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "popupmenu_menu"));
  app->popupmenu = widget;
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "aboutwindow"));
  app->about_window = widget;

  gtk_window_set_transient_for (GTK_WINDOW (app->about_window),
				GTK_WINDOW (app->window));

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "button_about_close"));
  g_signal_connect_swapped (widget, "clicked",
			    G_CALLBACK (gtk_widget_hide), app->about_window);
  return app;
}

void
fakeapp_create_gui (FakeApp * app)
{
  GdkColor color;
  GdkGC *gc;
  FakeButton *button;

  /* Configure the main window title and size */
  gtk_window_set_title (GTK_WINDOW (app->window), app->win_title);
  gtk_widget_set_size_request (app->fixed, app->device_width,
			       app->device_height);

  /* Move and set the size of the window for the Xnest. */
  gtk_widget_set_size_request (app->winnest,
			       app->device_display_width,
			       app->device_display_height);
  gtk_fixed_move (GTK_FIXED (app->fixed), app->winnest,
		  app->device_display_x, app->device_display_y);

  /* Set the device image */
  gtk_widget_set_size_request (app->back,
			       app->device_width,
			       app->device_height);
  gtk_fixed_move (GTK_FIXED (app->fixed), app->back, 0, 0);
  gtk_image_set_from_pixbuf (GTK_IMAGE (app->back), app->device_img);

  /* Now setup the buttons.  Should do this in realize but we can hack that */
  gtk_widget_realize (app->fixed);
  for (button = app->button_head; button; button = button->next)
    {
      GtkWidget *eventbox;
      button->normal_img =
	gdk_pixmap_new (app->fixed->window, button->width, button->height,
			-1);
      gdk_draw_pixbuf (GDK_DRAWABLE (button->normal_img), NULL,
		       app->device_img, button->x, button->y, 0, 0,
		       button->width, button->height, GDK_RGB_DITHER_NONE, 0,
		       0);
      button->image = gtk_image_new_from_pixmap (button->normal_img, NULL);
      gtk_widget_show (button->image);
      eventbox = gtk_event_box_new ();
      gtk_widget_show (eventbox);
      gtk_container_add (GTK_CONTAINER (eventbox), button->image);
      g_signal_connect (eventbox, "button-press-event",
			G_CALLBACK (button_press), button);
      g_signal_connect (eventbox, "button-release-event",
			G_CALLBACK (button_release), button);
      gtk_fixed_put (GTK_FIXED (app->fixed), eventbox, button->x, button->y);

      if (button->overlay)
	{
	  button->active_img =
	    gdk_pixmap_new (app->fixed->window, button->width, button->height,
			    -1);
	  g_assert (button->active_img);
	  gc = gdk_gc_new (GDK_DRAWABLE (button->active_img));
	  gdk_draw_drawable (button->active_img, gc, button->normal_img, 0, 0,
			     0, 0, -1, -1);
	  gdk_draw_pixbuf (button->active_img, gc, button->overlay, 0, 0, 0,
			   0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
	}
      else
	{
	  button->active_img =
	    gdk_pixmap_new (app->fixed->window, button->width, button->height,
			    -1);
	  gdk_draw_pixbuf (button->active_img, NULL, app->device_img,
			   button->x, button->y, 0, 0, button->width,
			   button->height, GDK_RGB_DITHER_NONE, 0, 0);
	  gc = gdk_gc_new (GDK_DRAWABLE (button->active_img));
	  gdk_color_parse ("yellow", &color);
	  gdk_gc_set_rgb_fg_color (gc, &color);
	  gdk_draw_rectangle (GDK_DRAWABLE (button->active_img), gc, FALSE, 0,
			      0, button->width - 1, button->height - 1);
	  g_object_unref (gc);
	}
    }
  gtk_widget_show (app->window);
}

static gboolean
key_event (GtkWidget * widget, GdkEventKey * event, FakeApp * app)
{
  if (app->xnest_window == 0)
    {
      g_warning ("Skipping event send, no window to send to");
    }
  else
    {
      XEvent xevent;
      xevent.xkey.type =
	(event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
      xevent.xkey.window = GDK_WINDOW_XWINDOW (app->winnest->window);
      xevent.xkey.root =
	GDK_WINDOW_XWINDOW (gdk_screen_get_root_window
			    (gdk_drawable_get_screen (app->winnest->window)));
      xevent.xkey.time = event->time;

      /* FIXME, the following might cause problems for non-GTK apps */

      xevent.xkey.x = 0;
      xevent.xkey.y = 0;
      xevent.xkey.x_root = 0;
      xevent.xkey.y_root = 0;
      xevent.xkey.state = event->state;
      xevent.xkey.keycode = event->hardware_keycode;
      xevent.xkey.same_screen = TRUE;

      gdk_error_trap_push ();
      XSendEvent (GDK_WINDOW_XDISPLAY
		  (app->winnest->window), app->xnest_window,
		  False, NoEventMask, &xevent);

      gdk_display_sync (gtk_widget_get_display (widget));

      if (gdk_error_trap_pop ())
	{
	  g_warning ("X error on XSendEvent");
	}
    }
  return TRUE;
}

gboolean
fakeapp_start_server (FakeApp * app)
{
  int pid;
  gchar winid[32];
  gchar exec_buf[2048];
  gchar **exec_vector = NULL;

  g_snprintf (winid, 32, "%li",
	      gdk_x11_drawable_get_xid (app->winnest->window));

  g_snprintf (exec_buf, 2048, "%s %s %s -parent %s",
	      "Xnest", app->xnest_dpy_name, app->xnest_bin_options, winid);

  /* Split the above up into something execv can digest */
  exec_vector = g_strsplit (exec_buf, " ", 0);

  signal (SIGCHLD, fakeapp_catch_sigchild);

  pid = fork ();
  switch (pid)
    {
    case 0:
      execv (app->xnest_bin_path, exec_vector);
      g_warning ("Failed to Launch %s\n", app->xnest_bin_path);
      exit (1);
    case -1:
      g_warning ("Failed to Launch %s\n", app->xnest_bin_path);
      break;
    default:
      g_strfreev (exec_vector);
      app->xnest_pid = pid;
      xnest_pid = pid;
    }

  if (pid == -1 || !keys_init (app))
    {
      g_warning ("'%s' Did not start correctly.", exec_buf);
      g_warning
	("Please restart with working --xnest-bin-options, --xnest-bin, options.");
      exit (1);
    }

  /* Disable the debug signal if we are not running Xephyr */
  if (strstr (app->xnest_bin_path, "Xephyr") == NULL)
    {
      gtk_widget_set_sensitive (app->debug_menu, FALSE);
    }
  else
    {
      gtk_widget_set_sensitive (app->debug_menu, TRUE);
    }

  if (app->start_cmd)
    {
      pid = fork ();
      switch (pid)
	{
	case 0:
	  setenv ("DISPLAY", app->xnest_dpy_name, 1);
	  execl ("/bin/sh", "sh", "-c", app->start_cmd, NULL);
	  g_warning ("Failed to Launch %s\n", app->start_cmd);
	  exit (1);
	case -1:
	  g_warning ("Failed to Launch %s\n", app->start_cmd);
	  break;
	default:
	  break;
	}
    }

  return FALSE;
}

static void
fakeapp_catch_sigchild (int sign)
{
  pid_t this_pid;
  this_pid = waitpid (-1, 0, WNOHANG);
  if (this_pid != xnest_pid)
    return;
  gtk_main_quit ();
}

gboolean
fakeapp_restart_server (FakeApp * app)
{
  if (app->xnest_pid > 0)
    {
      XCloseDisplay (app->xnest_dpy);

      signal (SIGCHLD, SIG_DFL);	/* start_server() will reset this */
      kill (app->xnest_pid, SIGTERM);
    }

  sleep (1);			/* give server a chance to quit  */

  return fakeapp_start_server (app);
}

int
main (int argc, char **argv)
{
  gchar* xnest_binary  = NULL;
  gchar* xnest_display = NULL;
  gchar* xnest_options = NULL;
  gchar* argv_device = NULL;
  gchar* title = NULL;
  GOptionContext* context;
  GOptionEntry entries[] = {
    {"device", 'd', 0, G_OPTION_ARG_STRING, &argv_device,
     N_("Device config file to use"), N_("DEVICE")},
    {"title", 't', 0, G_OPTION_ARG_STRING, &title,
     N_("Set the window title"), N_("TITLE")},
    {"xnest-bin", 'B', 0, G_OPTION_ARG_FILENAME, &xnest_binary,
     N_("Location of Xnest binary (default " XNEST_BIN")"), N_("BINARY")},
    {"xnest-bin-options", 'O', 0, G_OPTION_ARG_STRING, &xnest_options,
     N_("Command line options to pass to server (default '-ac')"), N_("OPTIONS")},
    {"xnest-dpy", 'D', 0, G_OPTION_ARG_STRING, &xnest_display,
     N_("Display String for Xnest to use (default '" XNEST_DPY "')"), N_("DISPLAY")},
    {NULL}
  };
  FakeApp *app;
  GError* error = NULL;
  char *device = PKGDATADIR "/ipaq4700.xml";

  context = g_option_context_new ("");
  g_option_context_set_description (context, _("Xoo is a graphical wrapper around Xnest"));
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, FALSE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      gchar* help = g_option_context_get_help (context, TRUE, NULL);
      g_option_context_free (context);
      g_printerr ("%s: %s\n%s\n", _("Error"), help, error->message);
      g_clear_error (&error);
      g_free (help);
      return 1;
    }
  g_clear_error (&error);
  g_option_context_free (context);

  app = fakeapp_new ();

  app->argv = argv;
  app->argc = argc;

#ifdef HAVE_GCONF
  /* Do this here so that command line argument override the GConf prefs */
  gconf_prefs_init (app);
#endif

  if (xnest_display)
    app->xnest_dpy_name = xnest_display;

  if (xnest_binary)
    app->xnest_bin_path = xnest_binary;

  if (xnest_options)
    app->xnest_bin_options = xnest_options;

  if (argv_device)
    device = argv_device;

  if (title)
    app->win_title = title;

  config_init (app, device);

  fakeapp_create_gui (app);

  g_idle_add ((GSourceFunc) fakeapp_start_server, app);
  gtk_main ();

  return 0;
}
