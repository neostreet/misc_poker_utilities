#include <stdio.h>
#include <string.h>

static char usage[] =
"usage: gen_twifs4 (-verbose) num_tournaments tournament_info_file\n";

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bVerbose;
  int num_tournaments;

  if ((argc < 3) || (argc > 4)) {
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

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  printf("@echo off\n");
  sscanf(argv[curr_arg],"%d",&num_tournaments);

  for (m = 0; m <= num_tournaments; m++) {
    for (n = 0; n <= num_tournaments - m; n++) {
      if (!bVerbose)
        printf("tournament_wif %s %d %d %d\n",argv[curr_arg+1],m,n,num_tournaments);
      else
        printf("tournament_wif -verbose %s %d %d %d\n",argv[curr_arg+1],m,n,num_tournaments);
    }
  }

  return 0;
}
