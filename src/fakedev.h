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

#ifndef _FAKEDEV_H
#define _FAKEDEV_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtkwidget.h>
#include <gtk/gtkimage.h>
#include <X11/extensions/XTest.h>

#include "config.h"

typedef struct FakeApp FakeApp;
typedef struct FakeButton FakeButton;

struct FakeApp
{
  GtkWidget *window;
  GtkWidget *fixed;
  GtkWidget *winnest;
  GtkWidget *popupmenu;
  GtkWidget *debug_menu;

  GtkWidget *prefs_window;
  GtkWidget *entry_display, *entry_server, *entry_options, *entry_start;
  GtkWidget *about_window;

  int            device_width;
  int            device_height;

  int            device_display_x;
  int            device_display_y;
  int            device_display_width;
  int            device_display_height;

  GdkPixbuf *device_img;

  /* Button stuff */
  
  FakeButton     *button_head; /* TODO: GList */

  struct timeval  key_rep_init_timeout;
  struct timeval  key_rep_timeout;

  /* Xnest  */

  Display        *xnest_dpy;
  Window         xnest_window;
  char           *xnest_dpy_name;
  char           *xnest_bin_path;
  char           *xnest_bin_options;
  pid_t            xnest_pid;
  
  char           *win_title;
  char           *start_cmd;

  int             argc; 
  char          **argv;
};

struct FakeButton
{
  FakeApp *app;
  GtkWidget *image;
  
  int            x,y;
  int            width,height;

  KeySym         keysym;

  GdkPixbuf *overlay;
  GdkPixmap *normal_img, *active_img;

  int            repeat;

  FakeButton  *next;
};


int
config_init(FakeApp *app, char *conf_file);

/* buttons */

FakeButton*
button_new(FakeApp *app, int x, int y, int width, int height, KeySym ks, int reepreat, GdkPixbuf *img);

FakeButton*
button_find_from_win(FakeApp *app, Window xevent_window_id);

void
button_press(GtkWidget *w, GdkEventButton *event, FakeButton *button);

void
button_release(GtkWidget *w, GdkEventButton *event, FakeButton *button);

void
button_activate(FakeApp *app, FakeButton *button);


/* keys */

#define KEYDOWN   1
#define KEYUP     2
#define KEYUPDOWN 3

int
keys_init(FakeApp *app);

void 
keys_send_key(FakeApp *app, KeySym ks, int key_direction);

#endif /* _FAKEDEV_H */
