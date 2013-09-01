#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

static char usage[] =
"usage: blue_distance2 (-terse) (-no_dates) (-in_sessions) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bTerse;
  bool bNoDates;
  bool bInSessions;
  FILE *fptr;
  int line_len;
  int line_no;
  char str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;
  int max_balance_ix;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bNoDates = false;
  bInSessions = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-no_dates"))
      bNoDates = true;
    else if (!strcmp(argv[curr_arg],"-in_sessions"))
      bInSessions = true;
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

  for ( ; ; ) {
    fscanf(fptr,"%s\t%d",str,&delta);

    if (feof(fptr))
      break;

    if (!line_no) {
      if (delta < 0) {
        max_balance = delta * -1;
        max_balance_ix = -1;
        balance = 0;
      }
      else {
        max_balance = delta;
        max_balance_ix = 0;
        balance = max_balance;
      }
    }
    else {
      balance += delta;

      if (balance > max_balance) {
        max_balance = balance;
        max_balance_ix = line_no;
      }
    }

    if (!bTerse) {
      if (!bNoDates) {
        if (!bInSessions)
          printf("%s\t%d\n",str,max_balance - balance);
        else
          printf("%s\t%d\n",str,line_no - max_balance_ix);
      }
      else {
        if (!bInSessions)
          printf("%d\n",max_balance - balance);
        else
          printf("%d\n",line_no - max_balance_ix);
      }
    }

    line_no++;
  }

  if (bTerse) {
    if (!bNoDates) {
      if (!bInSessions)
        printf("%s\t%d\n",str,max_balance - balance);
      else
        printf("%s\t%d\n",str,line_no - max_balance_ix);
    }
    else {
      if (!bInSessions)
        printf("%d\n",max_balance - balance);
      else
        printf("%d\n",line_no - max_balance_ix);
    }
  }

  fclose(fptr);

  return 0;
}
