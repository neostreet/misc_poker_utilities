#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

static char usage[] = "usage: blue_distance2 (-terse) (-no_dates) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int curr_arg;
  int bTerse;
  int bNoDates;
  FILE *fptr;
  int line_len;
  int line_no;
  char str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bNoDates = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-no_dates"))
      bNoDates = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
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

    if (!bTerse) {
      if (!bNoDates)
        printf("%s\t%d\n",str,max_balance - balance);
      else
        printf("%d\n",max_balance - balance);
    }
  }

  if (bTerse) {
    if (!bNoDates)
      printf("%s\t%d\n",str,max_balance - balance);
    else
      printf("%d\n",max_balance - balance);
  }

  fclose(fptr);

  return 0;
}
