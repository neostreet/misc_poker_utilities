#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: blue_distance2 (-terse) (-verbose) (-initial_bal) (-no_dates)\n"
"  (-only_blue) (-from_nonblue) (-in_sessions) (-is_blue) (-skyfall) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

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
  bool bSkyfall;
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

  if ((argc < 2) || (argc > 11)) {
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
  bSkyfall = false;
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
    else if (!strcmp(argv[curr_arg],"-skyfall"))
      bSkyfall = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bIsBlue && bSkyfall) {
    printf("can't specify both -is_blue and -skyfall\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;
  bPrevIsBlue = true;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%s\t%d",str,&delta);

    if (!line_no) {
      max_balance_ix = 0;

      if (initial_bal) {
        if (delta < 0) {
          max_balance = initial_bal;
          balance = initial_bal + delta;
        }
        else {
          max_balance = initial_bal + delta;
          balance = max_balance;
        }
      }
      else {
        max_balance = delta;
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
        if (!bOnlyBlue) {
          if (!bInSessions) {
            if (!bVerbose) {
              if (!bIsBlue) {
                if (!bSkyfall || ((delta < 0) && (line_no == max_balance_ix + 1))) {
                  printf("%d\t%s\n",
                  ((max_balance > 0) ? max_balance - balance : max_balance * -1),
                  line);
                }
              }
              else {
                printf("%d %d %d\t\%s\n",
                  ((max_balance > 0) ? max_balance - balance : max_balance * -1),
                  delta,
                  ((max_balance == balance) ? 1 : 0),line);
              }
            }
            else {
              if (!bIsBlue) {
                printf("%d (%d %d %d)\t%s\n",
                  ((max_balance > 0) ? max_balance - balance : max_balance * -1),
                  ((max_balance > 0) ? max_balance : 0),
                  balance,
                  ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1),
                  line);
              }
              else {
                printf("%d (%d %d %d) %d %d\t%s\n",
                  ((max_balance > 0) ? max_balance - balance : max_balance * -1),
                  ((max_balance > 0) ? max_balance : 0),
                  balance,
                  ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1),
                  delta,
                  ((max_balance == balance) ? 1 : 0),line);
              }
            }
          }
          else {
            printf("%d\t%s\n",
              ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1),
              line);
          }
        }
        else {
          if ((max_balance > 0) && (line_no == max_balance_ix)) {
            if (!bFromNonblue || !bPrevIsBlue) {
              if (!bVerbose)
                printf("%10d %s\n",max_balance,line);
              else
                printf("%10d %s (%d)\n",max_balance,line,line_no);
            }

            bPrevIsBlue = true;
          }
          else
            bPrevIsBlue = false;
        }
      }
      else {
        if (!bInSessions) {
          if (!bVerbose) {
            printf("%d\n",
              ((max_balance > 0) ? max_balance - balance : max_balance * -1));
          }
          else {
            printf("%d (%d %d %d)\n",
              ((max_balance > 0) ? max_balance - balance : max_balance * -1),
              ((max_balance > 0) ? max_balance : 0),
              balance,
              ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1));
          }
        }
        else {
          printf("%d\n",
            ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1));
        }
      }
    }

    line_no++;
  }

  if (bTerse) {
    if (!bNoDates) {
      if (!bInSessions) {
        printf("%d\t%s\n",
          ((max_balance > 0) ? max_balance - balance : max_balance * -1),
          line);
      }
      else {
        printf("%d\t%s\n",
          ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1),
          line);
      }
    }
    else {
      if (!bInSessions) {
        printf("%d\n",
          ((max_balance > 0) ? max_balance - balance : max_balance * -1));
      }
      else {
        printf("%d\n",
          ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1));
      }
    }
  }

  fclose(fptr);

  return 0;
}

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
{
  int chara;
  int local_line_len;

  local_line_len = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == '\n')
      break;

    if (local_line_len < maxllen - 1)
      line[local_line_len++] = (char)chara;
  }

  line[local_line_len] = 0;
  *line_len = local_line_len;
}
