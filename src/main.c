/* matchbox-nest - a graphical wrapper around xnest
 *
 *  Copyright 2004 Matthew Allum
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
#include <glade/glade-xml.h>
#include "fakedev.h"
#include "callbacks.h"
#include "prefs.h"

#define XNEST_BIN "/usr/X11R6/bin/Xnest"

/* Used by the signal handler to detect which child has died */
static pid_t xnest_pid;

static gboolean key_event (GtkWidget *widget, GdkEventKey *event, FakeApp *app);

static void
fakeapp_catch_sigchild(int sign);

FakeApp*
fakeapp_new(void)
{
  GladeXML *glade;
  FakeApp *app = g_new0(FakeApp, 1);

  app->xnest_dpy_name = ":1";
  app->xnest_bin_path = XNEST_BIN ;
  app->xnest_bin_options = strdup("-ac"); /* freed if changed */

  app->key_rep_init_timeout.tv_usec = 0;
  app->key_rep_init_timeout.tv_sec  = 1;

  app->key_rep_timeout.tv_usec      = 40000;
  app->key_rep_timeout.tv_sec       = 0;

  app->win_title                    = "matchbox-nest";

  glade = glade_xml_new (PKGDATADIR "/matchbox-nest.glade", NULL, NULL);
  g_assert (glade != NULL);
  
  glade_xml_signal_connect_data (glade, "on_send_signal_activate", (GCallback)on_send_signal_activate, app);
  glade_xml_signal_connect_data (glade, "on_quit_activate", (GCallback)on_quit_activate, app);
  glade_xml_signal_connect_data (glade, "on_about_activate", (GCallback)on_about_activate, app);
  glade_xml_signal_connect_data (glade, "on_window_destroy", (GCallback)on_window_destroy, app);
  glade_xml_signal_connect_data (glade, "on_popup_menu_show", (GCallback)on_popup_menu_show, app);
  glade_xml_signal_connect_data (glade, "on_show_decorations_toggle", (GCallback)on_show_decorations_toggle, app);
  glade_xml_signal_connect_data (glade, "on_delete_event_hide", (GCallback)on_delete_event_hide, app);
#if HAVE_GCONF
  glade_xml_signal_connect_data (glade, "on_preferences_activate", (GCallback)on_preferences_activate, app);
  glade_xml_signal_connect_data (glade, "on_prefs_apply_clicked", (GCallback)on_prefs_apply_clicked, app);
  glade_xml_signal_connect_data (glade, "on_prefs_cancel_clicked", (GCallback)on_prefs_cancel_clicked, app);
#else
  gtk_widget_hide (glade_xml_get_widget (glade, "preferences"));
#endif

  app->window = glade_xml_get_widget (glade, "window");
  app->fixed = glade_xml_get_widget (glade, "fixed");
  gtk_fixed_set_has_window (GTK_FIXED (app->fixed), TRUE);
  app->winnest = glade_xml_get_widget (glade, "winnest");
  g_signal_connect(app->window, "key-press-event", (GCallback)key_event, app);
  g_signal_connect(app->window, "key-release-event", (GCallback)key_event, app);

  app->prefs_window = glade_xml_get_widget (glade, "prefswindow");
  gtk_window_set_transient_for (GTK_WINDOW (app->prefs_window), GTK_WINDOW (app->window));
  app->entry_display = glade_xml_get_widget (glade, "entry_display");
  app->entry_server = glade_xml_get_widget (glade, "entry_server");
  app->entry_options = glade_xml_get_widget (glade, "entry_options");

  app->debug_menu = glade_xml_get_widget (glade, "send_signal");
  app->popupmenu = glade_xml_get_widget (glade, "popupmenu_menu");
  app->about_window = glade_xml_get_widget (glade, "aboutwindow");
  gtk_window_set_transient_for (GTK_WINDOW (app->about_window), GTK_WINDOW (app->window));
  g_signal_connect_swapped (glade_xml_get_widget (glade, "button_about_close"), "clicked", G_CALLBACK (gtk_widget_hide), app->about_window);
  
  return app;
}

void
fakeapp_create_gui(FakeApp *app)
{
  GdkPixmap *back;
  GdkColor color;
  GdkGC *gc;
  FakeButton *button;

  /* Configure the main window title and size */
  gtk_window_set_title (GTK_WINDOW (app->window), app->win_title);
  gtk_widget_set_size_request(app->window, app->device_width, app->device_height);

  /* Move and set the size of the window for the Xnest. */
  gtk_widget_set_size_request(app->winnest, 
                              app->device_display_width,
                              app->device_display_height);
  gtk_fixed_move (GTK_FIXED(app->fixed), app->winnest, 
                  app->device_display_x,
                  app->device_display_y);

  /* Force this widget to have X resources, so we can use it */
  gtk_widget_realize (app->fixed);

  /* Set the device image, by setting the back buffer of the widget */
  back = gdk_pixmap_new (app->fixed->window, app->device_width, app->device_height, -1);
  gc = gdk_gc_new (GDK_DRAWABLE (back));
  gdk_color_parse ("black", &color);
  gdk_gc_set_rgb_fg_color (gc, &color);
  gdk_draw_rectangle (GDK_DRAWABLE (back), gc, TRUE, 0, 0, app->device_width, app->device_height);
  gdk_draw_pixbuf (GDK_DRAWABLE (back), NULL, app->device_img, 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
  gdk_window_set_back_pixmap (app->fixed->window, back, FALSE);
  g_object_unref (gc);

  /* Now setup the buttons */
  for (button = app->button_head; button; button=button->next) 
    {
      GtkWidget *eventbox;
      button->normal_img = gdk_pixmap_new (app->fixed->window, button->width, button->height, -1);
      gdk_draw_pixbuf (GDK_DRAWABLE (button->normal_img), NULL, app->device_img, button->x, button->y, 0, 0, button->width, button->height, GDK_RGB_DITHER_NONE, 0, 0);
      button->image = gtk_image_new_from_pixmap (button->normal_img, NULL);
      gtk_widget_show (button->image);
      eventbox = gtk_event_box_new ();
      gtk_widget_show (eventbox);
      gtk_container_add (GTK_CONTAINER (eventbox), button->image);
      g_signal_connect (eventbox, "button-press-event", G_CALLBACK(button_press), button);
      g_signal_connect (eventbox, "button-release-event", G_CALLBACK(button_release), button);
      gtk_fixed_put (GTK_FIXED (app->fixed), eventbox, button->x, button->y);
      
      if (button->overlay) {
        button->active_img = gdk_pixmap_new (app->fixed->window, button->width, button->height, -1);
        g_assert (button->active_img);
        gc = gdk_gc_new (GDK_DRAWABLE (button->active_img));
        gdk_draw_drawable (button->active_img, gc, button->normal_img, 0, 0, 0, 0, -1, -1);
        gdk_draw_pixbuf (button->active_img, gc, button->overlay, 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
      } else {
        button->active_img = gdk_pixmap_new (app->fixed->window, button->width, button->height, -1);
        gdk_draw_pixbuf (button->active_img, NULL, app->device_img, button->x, button->y, 0, 0, button->width, button->height, GDK_RGB_DITHER_NONE, 0, 0);
        gc = gdk_gc_new (GDK_DRAWABLE (button->active_img));
        gdk_color_parse ("yellow", &color);
        gdk_gc_set_rgb_fg_color (gc, &color);
        gdk_draw_rectangle (GDK_DRAWABLE (button->active_img), gc, FALSE, 0, 0, button->width - 1, button->height - 1);
        g_object_unref (gc);
      }
    }
  gtk_widget_show (app->window);
}

static gboolean
key_event (GtkWidget *widget, GdkEventKey *event, FakeApp *app)
{
  if (app->xnest_window == 0) {
    g_warning("Skipping event send, no window to send to");
  } else {
    XEvent xevent;
    xevent.xkey.type = (event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
    xevent.xkey.window = GDK_WINDOW_XWINDOW (app->winnest->window);
    xevent.xkey.root = GDK_WINDOW_XWINDOW (gdk_screen_get_root_window (gdk_drawable_get_screen (app->winnest->window)));
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
    XSendEvent (GDK_WINDOW_XDISPLAY (app->winnest->window), app->xnest_window,
                False, NoEventMask, &xevent);
    gdk_display_sync (gtk_widget_get_display (widget));
    if (gdk_error_trap_pop ()) {
      g_warning("X error on XSendEvent");
    }
  }
  return TRUE;
}

gboolean
fakeapp_start_server(FakeApp *app)
{
  int pid;
  gchar winid[32];
  gchar exec_buf[2048];
  gchar **exec_vector = NULL;

  g_snprintf(winid, 32, "%li", 
	     gdk_x11_drawable_get_xid(app->winnest->window));

  g_snprintf(exec_buf, 2048, "%s %s %s -parent %s",
	     "Xnest", 
	     app->xnest_dpy_name, 
	     app->xnest_bin_options,
	     winid );

  /* Split the above up into something execv can digest */
  exec_vector = g_strsplit(exec_buf, " ", 0);

  signal(SIGCHLD, fakeapp_catch_sigchild);

  pid = fork();
  switch (pid) {
  case 0:
    execv(app->xnest_bin_path, exec_vector);
    g_warning( "Failed to Launch %s\n", app->xnest_bin_path);
    exit(1);
  case -1:
    g_warning("Failed to Launch %s\n", app->xnest_bin_path);
    break;
  default:
    g_strfreev(exec_vector);
    app->xnest_pid = pid;
    xnest_pid = pid;
  }

  if (pid == -1 || !keys_init(app))
    {
      g_warning ("'%s' Did not start correctly.", exec_buf);
      g_warning ("Please restart with working --xnest-bin-options, --xnest-bin, options.");
      exit(1);
    }

  /* Disable the debug signal if we are not running Xephyr */
  if (strstr(app->xnest_bin_path, "Xephyr") == NULL) {
    gtk_widget_set_sensitive (app->debug_menu, FALSE);
  } else {
    gtk_widget_set_sensitive (app->debug_menu, TRUE);
  }
  
  return FALSE;
}

static void
fakeapp_catch_sigchild(int sign)
{
  pid_t this_pid;
  this_pid = waitpid(-1, 0, WNOHANG);
  if (this_pid != xnest_pid) return;
  gtk_main_quit ();
}

gboolean
fakeapp_restart_server(FakeApp *app)
{
  if (app->xnest_pid > 0)
    {
      XCloseDisplay(app->xnest_dpy);

      signal(SIGCHLD, SIG_DFL); /* start_server() will reset this */
      kill (app->xnest_pid, SIGTERM);
    }

  sleep(1);  /* give server a chance to quit  */

  return fakeapp_start_server(app);
}

void 
usage(char *progname)
{
  fprintf(stderr, 
	  "%s " VERSION " usage:\n"
	  "--xnest-dpy,         -xd Display String for Xnest to use. ( default ':1')\n"
	  "--xnest-bin,         -xn  Location of Xnest binary ( default " XNEST_BIN ")\n"
	  "--xnest-bin-options  -xo  Command line opts to pass to server ( Default '-ac' )\n" 
	  "--title,             -t   Set the window title\n"     
	  "--device,            -d   Device config file to use\n"
	  "--help,              -h   Show this help\n",
	  progname);

  exit(1);
}

int 
main(int argc, char **argv)
{
  FakeApp *app;
  char    *device =   PKGDATADIR "/ipaq3800.xml"; 
  int      i;
  
  gtk_init(&argc, &argv);
  /* TODO: use popt */

  app = fakeapp_new();

#ifdef HAVE_GCONF
  /* Do this here so that command line argument override the GConf prefs */
  gconf_prefs_init(app);
#endif

  for (i = 1; i < argc; i++) {
    if (!strcmp ("--xnest-dpy", argv[i]) || !strcmp ("-xd", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      app->xnest_dpy_name = argv[i];
      continue;
    }
    if (!strcmp ("--xnest-bin", argv[i]) || !strcmp ("-xn", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      app->xnest_bin_path = argv[i];
      continue;
    }
    if (!strcmp ("--xnest-bin-options", argv[i]) 
	|| !strcmp ("-xo", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      app->xnest_bin_options = argv[i];
      continue;
    }

    if (!strcmp ("--device", argv[i]) || !strcmp ("-d", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      device = argv[i];
      continue;
    }
    if (!strcmp ("--title", argv[i]) || !strcmp ("-t", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      app->win_title = argv[i];
      continue;
    }

    if (!strcmp("--help", argv[i]) || !strcmp("-h", argv[i])) {
      usage(argv[0]);
    }

    usage(argv[0]);
  }

  config_init(app, device);

  fakeapp_create_gui(app);

  g_idle_add ((GSourceFunc)fakeapp_start_server, app);
  gtk_main();

  return 0;
}
