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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>		/* dirname() */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <expat.h>
#include "fakedev.h"

unsigned char *
config_load_file (const char *filename)
{
  struct stat st;
  FILE *fp;
  unsigned char *str;
  int len;
  char tmp[1024];

  strncpy (tmp, filename, 1024);

  if (stat (tmp, &st))
    {
      snprintf (tmp, 1024, "%s.xml", filename);
      if (stat (tmp, &st))
	{
	  snprintf (tmp, 1024, PKGDATADIR "/%s", filename);
	  if (stat (tmp, &st))
	    {
	      snprintf (tmp, 1024, PKGDATADIR "/%s.xml", filename);
	      if (stat (tmp, &st))
		return NULL;
	    }
	}
    }

  if (!(fp = fopen (tmp, "rb")))
    return NULL;

  /* Read in the file. */
  str = malloc (sizeof (char) * (st.st_size + 1));
  len = fread (str, 1, st.st_size, fp);
  if (len >= 0)
    str[len] = '\0';

  fclose (fp);

  /* change to the same directory as the conf file - for images */
  chdir (dirname (tmp));
  return str;
}

static const char *
config_get_val (char *key, const char **attr)
{
  int i = 0;

  while (attr[i] != NULL)
    {
      if (!strcmp (attr[i], key))
	return attr[i + 1];
      i += 2;
    }

  return NULL;
}

static GdkPixbuf *
config_load_image (FakeApp * app, const char *filename)
{
  GError *error = NULL;
  GdkPixbuf *pixbuf;
  char tmp[1024];
  struct stat st;

  strncpy (tmp, filename, 1024);

  if (stat (tmp, &st))
    {
      snprintf (tmp, 1024, PKGDATADIR "/%s", filename);
      if (stat (tmp, &st))
	{
	  return NULL;
	}
    }

  pixbuf = gdk_pixbuf_new_from_file (tmp, &error);
  if (error == NULL)
    {
      return pixbuf;
    }
  else
    {
      g_warning ("Could not load %s: %s", filename, error->message);
      g_error_free (error);
      return NULL;
    }
}


static int
config_handle_button_tag (FakeApp * app, const char **attr)
{
  const char *s;
  FakeButton *button;
  int x, y, width, height;
  KeySym keysym;
  int repeat = 1;
  GdkPixbuf *img = NULL;

  if ((s = config_get_val ("width", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'button' tag missing width param");
      return 0;
    }

  width = atoi (s);

  if ((s = config_get_val ("height", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'button' tag missing height param");
      return 0;
    }

  height = atoi (s);

  if ((s = config_get_val ("x", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'button' tag missing x param");
      return 0;
    }

  x = atoi (s);

  if ((s = config_get_val ("y", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'button' tag missing y param");
      return 0;
    }

  y = atoi (s);

  if ((s = config_get_val ("key", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'button' tag missing key param");
      return 0;
    }

  if ((keysym = XStringToKeysym (s)) == NoSymbol)
    {
      fprintf (stderr, "Error: Cant find keysym for '%s'", s);
      return 0;
    }

  /* optional attributes */

  if ((s = config_get_val ("img", attr)) != NULL)
    {
      if ((img = config_load_image (app, s)) == NULL)
	{
	  fprintf (stderr, "Error: Falied to load '%s' image.", s);
	}
    }

  if (((s = config_get_val ("repeat", attr)) != NULL)
      && !strcasecmp (s, "off"))
    repeat = 0;


  if (app->button_head == NULL)
    app->button_head = button_new (app, x, y, width, height,
				   keysym, repeat, img);
  else
    {
      for (button = app->button_head; button->next; button = button->next);
      button->next = button_new (app, x, y, width, height,
				 keysym, repeat, img);
    }

  return 1;
}

static int
config_handle_device_tag (FakeApp * app, const char **attr)
{
  /* 
     width, height, display_width, display_height, display_x, displa_y
   */
  const char *s;

  if ((s = config_get_val ("width", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'device' tag missing width param");
      return 0;
    }

  app->device_width = atoi (s);

  if ((s = config_get_val ("height", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'device' tag missing height param");
      return 0;
    }

  app->device_height = atoi (s);

  if ((s = config_get_val ("display_width", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'device' tag missing display_width param");
      return 0;
    }

  app->device_display_width = atoi (s);

  if ((s = config_get_val ("display_height", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'device' tag missing display_height param");
      return 0;
    }

  app->device_display_height = atoi (s);

  if ((s = config_get_val ("display_x", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'device' tag missing display_x param");
      return 0;
    }

  app->device_display_x = atoi (s);

  if ((s = config_get_val ("display_y", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'device' tag missing display_x param");
      return 0;
    }

  app->device_display_y = atoi (s);

  if ((s = config_get_val ("img", attr)) == NULL)
    {
      fprintf (stderr, "Error: 'device' tag missing img param");
      return 0;
    }

  if ((app->device_img = config_load_image (app, s)) == NULL)
    {
      fprintf (stderr, "Error: Falied to load '%s' image.", s);
      return 0;
    }

  return 1;
}

static void
config_xml_start_cb (void *data, const char *tag, const char **attr)
{
  FakeApp *app = (FakeApp *) data;

  if (!strcmp (tag, "button"))
    {
      if (!config_handle_button_tag (app, attr))
	{
	  fprintf (stderr, "Giving up.\n");
	  exit (1);
	}
    }
  else if (!strcmp (tag, "device"))
    {
      if (!config_handle_device_tag (app, attr))
	{
	  fprintf (stderr, "Giving up.\n");
	  exit (1);
	}
    }
}


int
config_init (FakeApp * app, char *conf_file)
{
  unsigned char *data;
  XML_Parser p;

  if ((data = config_load_file (conf_file)) == NULL)
    {
      fprintf (stderr, "Couldn't find '%s' device config file\n", conf_file);
      exit (1);
    }

  p = XML_ParserCreate (NULL);

  if (!p)
    {
      fprintf (stderr, "Couldn't allocate memory for XML parser\n");
      exit (1);
    }

  XML_SetElementHandler (p, config_xml_start_cb, NULL);
  /* XML_SetCharacterDataHandler(p, chars); */

  XML_SetUserData (p, (void *) app);

  if (!XML_Parse (p, data, strlen (data), 1))
    {
      fprintf (stderr, "XML Parse error at line %d:\n%s\n",
	       XML_GetCurrentLineNumber (p),
	       XML_ErrorString (XML_GetErrorCode (p)));
      exit (1);
    }


  return 1;
}

void
config_reinit (FakeApp * app, char *conf_file)
{
  /* Save these so we can figure out if server needs restarting */

  /*
     int prev_device_display_x      = app->device_display_x;
     int prev_device_display_y      = app->device_display_y;
     int prev_device_display_width  = app->device_display_width;
     int prev_device_display_height = app->device_display_height;
   */

  /* XXX TODO: Free up our old structure images etc XXX */

  /* Reload UI - hard ? */

}
