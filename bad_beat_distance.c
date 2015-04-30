#include <stdio.h>

int main(int argc,char **argv)
{
  int invested;
  int pot_size;
  int win_pct;
  int work;
  double expected_value;

  if (argc != 4) {
    printf("usage: bad_beat_distance invested pot_size win_pct");
    return 1;
  }

  sscanf(argv[1],"%d",&invested);
  sscanf(argv[2],"%d",&pot_size);
  sscanf(argv[3],"%d",&win_pct);

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

  printf("%lf\n",expected_value + (double)invested);

  return 0;
}
