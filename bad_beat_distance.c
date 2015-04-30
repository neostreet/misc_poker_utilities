#include <stdio.h>
#include <string.h>

static char usage[] =
"usage: bad_beat_distance (-expected_value) invested pot_size win_pct";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bExpectedValue;
  int invested;
  int pot_size;
  int win_pct;
  int work;
  double expected_value;

  if ((argc < 4) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-expected_value"))
      bExpectedValue = true;
    else
      break;
  }

  if (argc - curr_arg != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[curr_arg],"%d",&invested);
  sscanf(argv[curr_arg+1],"%d",&pot_size);
  sscanf(argv[curr_arg+2],"%d",&win_pct);

  if (pot_size < invested * 2) {
    printf("invested is greater than half of the pot size\n");
    return 2;
  }

  if ((win_pct < 0) || (win_pct > 100)) {
    printf("win_pct must be >= 0 and <= 100\n");
    return 3;
  }

  work = 100 * invested * -1 + win_pct * pot_size;

  expected_value = (double)work / (double)100;

  if (bExpectedValue)
    printf("%lf\n",expected_value);
  else
    printf("%lf\n",expected_value + (double)invested);

  return 0;
}
