#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

static char usage[] =
"usage: blue_summary filename\n";
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
  int blue_count;
  double dwork;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  blue_count = 0;

  for ( ; ; ) {
    fscanf(fptr,"%s\t%d",str,&delta);

    if (feof(fptr))
      break;

    if (!line_no) {
      if (delta < 0) {
        max_balance = delta * -1;
        balance = 0;
      }
      else {
        max_balance = delta;
        balance = max_balance;
        blue_count++;
      }
    }
    else {
      balance += delta;

      if (balance > max_balance) {
        max_balance = balance;
        blue_count++;
      }
    }

    line_no++;
  }

  fclose(fptr);

  dwork = (double)blue_count / (double)line_no;

  printf("%6d %6d %7.4lf\n",blue_count,line_no,dwork);

  return 0;
}
