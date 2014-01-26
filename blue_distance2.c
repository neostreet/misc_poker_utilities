#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

static char usage[] =
"usage: blue_distance2 (-terse) (-verbose) (-initial_bal) (-no_dates)\n"
"  (-only_blue) (-from_nonblue) (-in_sessions) (-is_blue) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  bool bNoDates;
  bool bOnlyBlue;
  bool bFromNonblue;
  bool bInSessions;
  bool bIsBlue;
  bool bPrevIsBlue;
  int initial_bal;
  FILE *fptr;
  int line_len;
  int line_no;
  char str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;
  int max_balance_ix;

  if ((argc < 2) || (argc > 10)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  bNoDates = false;
  bOnlyBlue = false;
  bFromNonblue = false;
  bInSessions = false;
  bIsBlue = false;
  initial_bal = 0;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strncmp(argv[curr_arg],"-initial_bal",12))
      sscanf(&argv[curr_arg][12],"%d",&initial_bal);
    else if (!strcmp(argv[curr_arg],"-no_dates"))
      bNoDates = true;
    else if (!strcmp(argv[curr_arg],"-only_blue"))
      bOnlyBlue = true;
    else if (!strcmp(argv[curr_arg],"-from_nonblue"))
      bFromNonblue = true;
    else if (!strcmp(argv[curr_arg],"-in_sessions"))
      bInSessions = true;
    else if (!strcmp(argv[curr_arg],"-is_blue"))
      bIsBlue = true;
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
  bPrevIsBlue = true;

  for ( ; ; ) {
    fscanf(fptr,"%s\t%d",str,&delta);

    if (feof(fptr))
      break;

    if (!line_no) {
      if (initial_bal) {
        if (delta < 0) {
          max_balance = initial_bal;
          max_balance_ix = -1;
          balance = initial_bal + delta;
        }
        else {
          max_balance = initial_bal + delta;
          max_balance_ix = 0;
          balance = max_balance;
        }
      }
      else {
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
        if (!bOnlyBlue) {
          if (!bInSessions) {
            if (!bVerbose) {
              if (!bIsBlue)
                printf("%s\t%d\n",str,max_balance - balance);
              else {
                printf("%s\t%d %d %d\n",str,max_balance - balance,delta,
                  ((max_balance == balance) ? 1 : 0));
              }
            }
            else {
              if (!bIsBlue) {
                printf("%s\t%d (%d %d)\n",str,max_balance - balance,
                  max_balance,balance);
              }
              else {
                printf("%s\t%d (%d %d) %d %d\n",str,max_balance - balance,
                  max_balance,balance,delta,
                  ((max_balance == balance) ? 1 : 0));
              }
            }
          }
          else
            printf("%s\t%d\n",str,line_no - max_balance_ix);
        }
        else {
          if (line_no == max_balance_ix) {
            if (!bFromNonblue || !bPrevIsBlue) {
              if (!bVerbose)
                printf("%10d %10d %s\n",delta,max_balance,str);
              else
                printf("%10d %10d %s (%d)\n",delta,max_balance,str,line_no);
            }

            bPrevIsBlue = true;
          }
          else
            bPrevIsBlue = false;
        }
      }
      else {
        if (!bInSessions) {
          if (!bVerbose)
            printf("%d\n",max_balance - balance);
          else {
            printf("%d (%d %d)\n",max_balance - balance,
              max_balance,balance);
          }
        }
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
