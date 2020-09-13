/* Copyright (C) 2020 Lars Brinkhoff <lars@nocrew.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "lmfs.h"

#define TYPE_PRE  0
#define TYPE_DIR  1
#define TYPE_FIL  2

static const char *type_name[] =
  {
    "Prelude",
    "Directory",
    "File"
  };

static char *buffer;
static char filename[1024];

static void read_info (int type)
{
  char *p, *x;
  int n;

  if (verbose >= 3)
    {
      fprintf (stderr, "%s:", type_name[type]);
      if (type == TYPE_PRE)
        fputc ('\n', stderr);
    }

  x = strchr (buffer, 0215);
  *x = 0;
  p = x + 1;
  if ((x = strchr (buffer, ' ')) != NULL)
    {
      *x++ = 0;
      strncpy (filename, x, sizeof filename);
      if (verbose >= 3)
        {
          n = fprintf (stderr, " %s\n", x);
        }
      else if (type == TYPE_FIL)
        {
          if (verbose >= 1)
            n = fprintf (stderr, "%s", x);
          if (verbose != 2)
            fputc ('\n', stderr);
        }
    }
  for (;;)
    {
      x = strchr (p, 0215);
      *x = 0;
      if (strncmp (p, "END", 3) == 0)
        break;
      if (type == TYPE_FIL)
        {
          if (strncmp (p, "CREATION-DATE", 13) == 0 && verbose == 2)
            fprintf (stderr, " %*s\n", 75 - n, p + 14);
        }
      if (verbose >= 3)
        fprintf (stderr, "  %s\n", p);
      p = x + 1;
    }
}

static void read_eof (FILE *f)
{
  int n;
  while ((n = tape_get_record (f, &buffer)) > 0)
    {
      free (buffer);
    }
}

static void read_prelude (FILE *f, int n)
{
  read_info (TYPE_PRE);
  free (buffer);
  read_eof (f);
}

static void read_directory (FILE *f, int n)
{
  read_info (TYPE_DIR);
  free (buffer);
  read_eof (f);
}

static char *weenix (char *name)
{
  int i, n;
  n = strlen (name);
  for (i = 0; i < n; i++)
    if (name[i] == '>')
      name[i] = '/';
  return name;
}

static void mkdirs (char *name, char *p)
{
  char *x = strchr (p, '/');
  char tmp;

  if (x == NULL)
    return;

  tmp = *x;
  *x = 0;
  if (mkdir (name, 0777) == -1 && errno != EEXIST)
    fprintf (stderr, "Error making %s: %s\n", name, strerror (errno));
  *x = tmp;

  mkdirs (name, x + 1);
}

static void read_contents (FILE *f, FILE *out)
{
  int n;
  while ((n = tape_get_record (f, &buffer)) > 0)
    {
#if 0
      {
        int i;
        fprintf (stderr, "{");
        for (i = 0; i < n; i++)
          fprintf (stderr, "%c", buffer[i]);
        fprintf (stderr, "}");
      }
#endif

      fwrite (buffer, 1, n, out);
      free (buffer);
    }
}

static void read_file (FILE *f, int n)
{
  FILE *out;
  read_info (TYPE_FIL);
  free (buffer);

  if (extract)
    {
      char *fn = weenix (filename);
      if (fn[0] == '/')
        fn++;
      mkdirs (fn, fn);
      out = fopen (fn, "wb");
      if (out == NULL)
        fprintf (stderr, "Error opening %s\n", fn);
      read_contents (f, out);
      fclose (out);
    }
  else
    read_eof (f);
}

void read_tape (FILE *f)
{
  int n;
  n = tape_get_record (f, &buffer);
  if (n == 0)
    exit (0);
  else if (strncmp (buffer, "PRELUDE", 7) == 0)
    read_prelude (f, n);
  else if (strncmp (buffer, "DIRECTORY", 9) == 0)
    read_directory (f, n);
  else if (strncmp (buffer, "FILE", 4) == 0)
    read_file (f, n);
  else
    fprintf (stderr, "UNKNOWN: %s\n", buffer);
}
