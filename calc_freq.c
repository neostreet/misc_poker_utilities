#include <stdio.h>
#include <string.h>

static char usage[] =
"usage: calc_freq (-verbose) instances hands periodicity\n";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int instances;
  int hands;
  double periodicity;
  double work;

  if ((argc < 4) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 3) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&instances);
  sscanf(argv[curr_arg+1],"%d",&hands);
  sscanf(argv[curr_arg+2],"%lf",&periodicity);

  work = (double)instances * periodicity;
  work /= (double)hands;

  if (!bVerbose)
    printf("%lf\n",work);
  else
    printf("%lf (%d %d %lf)\n",work,instances,hands,periodicity);

  return 0;
}
