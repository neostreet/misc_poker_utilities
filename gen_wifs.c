#include <stdio.h>
#include <string.h>

#define NUM_PLACES 6

static char usage[] =
"usage: gen_wifs (-verbose) (-right_justify) tournament_info_file\n";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bRightJustify;
  int m;
  int n;

  if ((argc < 2) || (argc > 4)) {
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

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  printf("@echo off\n\n");

  for (m = 0; m <= NUM_PLACES; m++) {
    for (n = 0; n <= NUM_PLACES; n++) {
      if (m + n <= NUM_PLACES) {
        if (!bVerbose) {
          if (!bRightJustify)
            printf("tournament_wif %s %d %d %d\n",
              argv[curr_arg],m,n,NUM_PLACES);
          else {
            printf("tournament_wif -right_justify %s %d %d %d\n",
              argv[curr_arg],m,n,NUM_PLACES);
          }
        }
        else {
          printf("tournament_wif -verbose %s %d %d %d\n",
            argv[curr_arg],m,n,NUM_PLACES);
        }
      }
    }
  }

  return 0;
}
