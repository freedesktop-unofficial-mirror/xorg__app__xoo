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
#include <stdio.h>
#include <glib/gmessages.h>
#include <gtk/gtkimage.h>
#include "fakedev.h"

FakeButton*
button_new(FakeApp *app, 
	   int      x, 
	   int      y, 
	   int      width, 
	   int      height, 
	   KeySym   ks,
	   int      repeat,
	   GdkPixbuf *img)
{
  FakeButton *button = malloc(sizeof(FakeButton));
  memset(button, 0, sizeof(FakeButton));

  button->app = app;
  button->x = x;
  button->y = y;
  button->height = height;
  button->width  = width;
  button->keysym = ks;
  button->overlay    = img;
  button->repeat = repeat;

  if (img)
    {
      if (gdk_pixbuf_get_width(img) != button->width
	  || gdk_pixbuf_get_height(img) != button->height)
	{
	  fprintf(stderr, 
		  "** warning: Button image size does not equal defined button size.\n"
		  );

	}
    }

  /* keysyms */

  return button;
}

#if 0
FakeButton*
button_find_from_win(FakeApp *app, Window xevent_window_id)
{
  FakeButton *button;

  for (button = app->button_head; button; button=button->next) 
    if (button->win == xevent_window_id)
      return button;

  return NULL;

} 
#endif

void button_press(GtkWidget *w, GdkEventButton *event, FakeButton *button) {
  g_return_if_fail (button != NULL);
  gtk_image_set_from_pixmap (GTK_IMAGE(button->image), button->active_img, NULL);
  keys_send_key(button->app, button->keysym, KEYDOWN);
}

void button_release(GtkWidget *w, GdkEventButton *event, FakeButton *button) {
  g_return_if_fail (button != NULL);
  gtk_image_set_from_pixmap (GTK_IMAGE(button->image), button->normal_img, NULL);
  keys_send_key(button->app, button->keysym, KEYUP);
}

void
button_activate(FakeApp *app, FakeButton *button)
{
  ;
}
