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

#include "config.h"
#include <X11/extensions/XTest.h>
#include "fakedev.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

static int
keys_xnest_connect(FakeApp *app)
{
  int trys = 10;

  do 
    {
      if (--trys < 0) return 0;

      sleep(1); /* Sleep for a second to give Xnest a chance to start */

      app->xnest_dpy = XOpenDisplay(app->xnest_dpy_name);
      {
        int num_children;
        Window root, parent;
        Window *windows;
        XQueryTree (GDK_WINDOW_XDISPLAY (app->winnest->window), GDK_WINDOW_XWINDOW(app->winnest->window), &root, &parent, &windows, &num_children);
        if (num_children != 0) {
          app->xnest_window = windows[0];
          XFree (windows);
        } else {
          app->xnest_window = 0;
        }
      }
    }
  while ( app->xnest_dpy == NULL );

  if (app->xnest_window == 0)
    {
      g_warning ("X Server failed to start.");
      return 0;
    }

  return 1;
}

void 
keys_send_key(FakeApp *app, KeySym keysym, int key_direction)
{
  KeyCode character;

  character = XKeysymToKeycode(app->xnest_dpy, keysym);

  switch (key_direction)
    {
    case KEYDOWN:
      XTestFakeKeyEvent(app->xnest_dpy, (unsigned int) character, True, 0);
      break;
    case KEYUP:
      XTestFakeKeyEvent(app->xnest_dpy, (unsigned int) character, False, 0);
      break;
    case KEYUPDOWN:
      XTestFakeKeyEvent(app->xnest_dpy, (unsigned int) character, False, 0);
      XTestFakeKeyEvent(app->xnest_dpy, (unsigned int) character, True, 0);
      break;
    }

  /* Make sure the event gets sent */
  XFlush(app->xnest_dpy);

}

int
keys_init(FakeApp *app)
{
  int event, error;
  int major, minor;

  if (keys_xnest_connect(app))
    {
      if (!XTestQueryExtension(app->xnest_dpy, &event, &error, &major, &minor))
	{
	  g_warning ("XTest extension not supported on server \"%s\"\n.", DisplayString(app->xnest_dpy));
	  return 0;
	}

      return 1;

    }

  g_warning ("Failed to connect to X Server Display.");

  return 0;
}
