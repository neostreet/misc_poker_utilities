#include <stdio.h>
#include <stdlib.h>

static char usage[] = "usage: gen_twifs filename\n";
static char couldnt_open[] = "couldn't open %s\n";

#define BUF_LEN 1024
static char buf[BUF_LEN];

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_no;
  int work;
  int firsts;
  int seconds;
  int total;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  total = 0;

  for ( ; ; ) {
    fscanf(fptr,"%d",&work);

    if (feof(fptr))
      break;

    line_no++;

    if (line_no == 1)
      firsts = work;
    else if (line_no == 2)
      seconds = work;

    total += work;
  }

  fclose(fptr);

  if (line_no != 6) {
    printf("found %d lines; expected 6\n",line_no);
    return 1;
  }

  sprintf(buf,"tournament_wif /cygdrive/c/aidan/pokerstars/50000a %d %d %d",firsts,seconds,total);
  system(buf);
  sprintf(buf,"tournament_wif /cygdrive/c/aidan/pokerstars/50000b %d %d %d",firsts,seconds,total);
  system(buf);

  return 0;
}
