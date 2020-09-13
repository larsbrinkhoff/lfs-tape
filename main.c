#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lmfs.h"

int verbose = 0;
int extract = 0;

static void
usage (const char *x)
{
  fprintf (stderr, "Usage: %s ...\n", x);
  exit (1);
}

int
main (int argc, char **argv)
{
  FILE *f = NULL;
  int opt;

  while ((opt = getopt (argc, argv, "vt:x:")) != -1)
    {
      switch (opt)
	{
	case 'v':
	  verbose++;
	  break;
	case 't':
	  verbose++;
          if (f)
            usage (argv[0]);
          f = fopen (optarg, "rb");
	  break;
	case 'x':
          if (f)
            usage (argv[0]);
          f = fopen (optarg, "rb");
          extract = 1;
	  break;
        default:
          usage (argv[0]);
        }
    }

  if (optind != argc)
    usage (argv[0]);

  if (f == NULL)
    usage (argv[0]);

  for (;;)
    read_tape (f);

  return 0;
}
