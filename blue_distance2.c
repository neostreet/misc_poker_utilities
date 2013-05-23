#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

static char usage[] = "usage: blue_distance2 filename\n";
static char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  char str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  balance = 0;

  for ( ; ; ) {
    fscanf(fptr,"%s\t%d",str,&delta);

    if (feof(fptr))
      break;

    line_no++;

    balance += delta;

    if ((line_no == 1) || (balance > max_balance))
      max_balance = balance;

    printf("%s\t%d\n",str,max_balance - balance);
  }

  fclose(fptr);

  return 0;
}
