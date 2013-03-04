#include <stdio.h>
#include <string.h>

#define NUM_PLACES 6

static char usage[] =
"usage: gen_wifs (-summary) tournament_info_file\n";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bSummary;
  int m;
  int n;
  int o;
  int p;
  int q;
  int r;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bSummary = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-summary"))
      bSummary = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  printf("@echo off\n\n");

  if (bSummary) {
    for (m = 0; m <= NUM_PLACES; m++) {
      for (n = 0; n <= NUM_PLACES; n++) {
        if (m + n <= NUM_PLACES) {
          printf("tournament_wif -right_justify %s %d %d %d\n",
            argv[curr_arg],m,n,NUM_PLACES);
        }
      }
    }
  }
  else {
    for (m = 0; m <= NUM_PLACES; m++) {
      for (n = 0; n <= NUM_PLACES; n++) {
        for (o = 0; o <= NUM_PLACES; o++) {
          for (p = 0; p <= NUM_PLACES; p++) {
            for (q = 0; q <= NUM_PLACES; q++) {
              for (r = 0; r <= NUM_PLACES; r++) {
                if (m + n + o + p + q + r <= NUM_PLACES) {
                  printf("tournament_wif -right_justify %s %d %d %d\n",
                    argv[curr_arg],m,n,NUM_PLACES);
                }
              }
            }
          }
        }
      }
    }
  }

  return 0;
}
