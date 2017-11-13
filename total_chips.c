#include <stdio.h>

int main(int argc,char **argv)
{
  int entries;
  int rebuys;
  int add_ons;
  int total_chips;

  if (argc != 4) {
    printf("usage: total_chips entries rebuys add_ons\n");
    return 1;
  }

  sscanf(argv[1],"%d",&entries);
  sscanf(argv[2],"%d",&rebuys);
  sscanf(argv[3],"%d",&add_ons);

  total_chips = (entries + rebuys) * 1500 + (add_ons * 2000);

  printf("%d %d %d %d\n",total_chips,entries,rebuys,add_ons);

  return 0;
}
