#include <stdio.h>

#define NUM_UNIQUE_HANDS 1326

int main(int argc,char **argv)
{
  int instances;
  int hands;
  int instances_per_1326;
  double work;

  if (argc != 4) {
    printf("usage: calc_freq instances hands instances_per_1326\n");
    return 1;
  }

  sscanf(argv[1],"%d",&instances);
  sscanf(argv[2],"%d",&hands);
  sscanf(argv[3],"%d",&instances_per_1326);

  work = (double)instances * (double)NUM_UNIQUE_HANDS;
  work /= (double)hands;
  work /= (double)instances_per_1326;

  printf("%lf\n",work);

  return 0;
}
