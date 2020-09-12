/* Copyright (C) 2009, 2020 Lars Brinkhoff <lars@nocrew.org>

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
#include <stdlib.h>

static int
get_byte (FILE *f)
{
  int c = fgetc (f);
  return c == EOF ? 0 : c;
}

static int get_reclen (FILE *f)
{
  return get_byte (f) |
    (get_byte (f) << 8) |
    (get_byte (f) << 16) |
    (get_byte (f) << 24);
}

static void write_reclen (FILE *f, int n)
{
  fputc (n & 0377, f);
  fputc ((n >> 8) & 0377, f);
  fputc ((n >> 16) & 0377, f);
  fputc ((n >> 24) & 0377, f);
}

int tape_get_record (FILE *f, char **buffer)
{
  int i, x, reclen;
  char *p;

  reclen = get_reclen (f);
  if (reclen == 0)
    return 0;
  
  *buffer = malloc (reclen);
  if (*buffer == NULL)
    {
      fprintf (stderr, "Out of memory.\n");
      exit (1);
    }

  for (i = 0, p = *buffer; i < reclen; i++)
    *p++ = get_byte (f);

  /* First try the E-11 tape format. */
  x = get_reclen (f);
  if (x != reclen)
    {
      /* Next try the SIMH tape format. */
      if (reclen & 1)
	x = (x >> 8) + (get_byte (f) << 24);

      if (x != reclen)
	{
	  fprintf (stderr, "Error in tape image format.\n"
		   "%d != %d\n", reclen, x);
	  exit (1);
	}
    }

  if (reclen == 0)
    {
      free (*buffer);
    }

  return reclen;
}

void tape_write_record (FILE *f, char *buffer, int n)
{
  int i;

  /* To write a tape record in the SIMH tape image format, first write
     a 32-bit record length, then data frames, then the length again.
     For PDP-10 36-bit data, the data words are written in the "core
     dump" format.  One word is written as five 8-bit frames, with
     four bits unused in the last frame. */

  write_reclen (f, n);

  /* A record of length zero is a tape mark, and the length is only
     written once. */
  if (n == 0)
    return;
  
  for (i = 0; i < n; i++)
    fputc (*buffer++, f);

  /* Pad out to make the record data an even number of octets. */
  if (n & 1)
    fputc (0, f);

  write_reclen (f, n);
}
