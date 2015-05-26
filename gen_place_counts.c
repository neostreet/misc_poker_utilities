#include <stdio.h>

static char usage[] = "usage: gen_place_counts num_sit_and_gos\n";

int main(int argc,char **argv)
{
  int m;
  int n;
  int num_sit_and_gos;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&num_sit_and_gos);

  for (m = 0; m <= num_sit_and_gos; m++) {
    for (n = 0; n <= num_sit_and_gos - m; n++) {
      printf("%d %d %d\n",m,n,num_sit_and_gos);
    }
  }

  return 0;
}
