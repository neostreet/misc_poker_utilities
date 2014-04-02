#include <stdio.h>
#include <string.h>

static char usage[] =
"usage: gen_wifs (-verbose) (-right_justify) tournament_info_file num_tournaments\n";

int main(int argc,char **argv)
{
  int curr_arg;
  int num_tournaments;
  bool bVerbose;
  bool bRightJustify;
  int m;
  int n;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bRightJustify = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-right_justify"))
      bRightJustify = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg+1],"%d",&num_tournaments);

  printf("@echo off\n\n");

  for (m = 0; m <= num_tournaments; m++) {
    for (n = 0; n <= num_tournaments; n++) {
      if (m + n <= num_tournaments) {
        if (!bVerbose) {
          if (!bRightJustify)
            printf("tournament_wif %s %d %d %d\n",
              argv[curr_arg],m,n,num_tournaments);
          else {
            printf("tournament_wif -right_justify %s %d %d %d\n",
              argv[curr_arg],m,n,num_tournaments);
          }
        }
        else {
          printf("tournament_wif -verbose %s %d %d %d\n",
            argv[curr_arg],m,n,num_tournaments);
        }
      }
    }
  }

  return 0;
}
