#include <stdio.h>
#include <stdlib.h>
#include "lmfs.h"

static void
usage (const char *x)
{
  fprintf (stderr, "Usage: %s ...\n", x);
  exit (1);
}

int
main (int argc, char **argv)
{
  FILE *f;

  if (argc != 2)
    usage (argv[0]);

  f = fopen (argv[1], "rb");
  if (f == NULL)
    {
      fprintf (stderr, "error\n");
      exit (1);
    }

  for (;;)
    read_tape (f);

  return 0;
}
