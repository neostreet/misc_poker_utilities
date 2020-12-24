#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: blue_distance2 (-terse) (-verbose) (-sum)\n"
"  (-initial_balbal) (-initial_max_balbal) (-initial_max_blue_distdist\n"
"  (-no_dates) (-only_blue) (-from_nonblue) (-in_sessions) (-is_blue) (-skyfall)\n"
"  (-no_input_dates) (-only_max) (-runtot) (-truncate) (-insert)\n"
"  (-geval) (-no_distance) (-blue_leap) (-debug)\n"
"  (-is_max_blue_distance) (-pct) (-new_year) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  bool bSum;
  bool bNoDates;
  bool bOnlyBlue;
  bool bFromNonblue;
  bool bInSessions;
  bool bIsBlue;
  bool bSkyfall;
  bool bNoInputDates;
  bool bOnlyMax;
  bool bRuntot;
  bool bTruncate;
  bool bInsert;
  int ge_val;
  bool bNoDistance;
  bool bBlueLeap;
  bool bDebug;
  bool bPrevIsBlue;
  bool bIsMaxBlueDistance;
  bool bPct;
  bool bNewYear;
  int initial_bal;
  int initial_max_bal;
  int initial_max_blue_dist;
  FILE *fptr;
  int line_len;
  int line_no;
  int last_blue_line_no;
  char str[MAX_STR_LEN];
  char prev_str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;
  int max_balance_ix;
  int blue_distance;
  int max_blue_distance;
  int blue_leap;
  int new_max_count;
  int same_max_count;

  if ((argc < 2) || (argc > 26)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  bSum = false;
  bNoDates = false;
  bOnlyBlue = false;
  bFromNonblue = false;
  bInSessions = false;
  bIsBlue = false;
  bSkyfall = false;
  bNoInputDates = false;
  bOnlyMax = false;
  bRuntot = false;
  bTruncate = false;
  bInsert = false;
  ge_val = -1;
  initial_bal = 0;
  initial_max_bal = 0;
  initial_max_blue_dist = 0;
  bNoDistance = false;
  bBlueLeap = false;
  bDebug = false;
  bIsMaxBlueDistance = false;
  bPct = false;
  bNewYear = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-sum"))
      bSum = true;
    else if (!strncmp(argv[curr_arg],"-initial_bal",12))
      sscanf(&argv[curr_arg][12],"%d",&initial_bal);
    else if (!strncmp(argv[curr_arg],"-initial_max_bal",16))
      sscanf(&argv[curr_arg][16],"%d",&initial_max_bal);
    else if (!strncmp(argv[curr_arg],"-initial_max_blue_dist",22))
      sscanf(&argv[curr_arg][22],"%d",&initial_max_blue_dist);
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
    else if (!strcmp(argv[curr_arg],"-no_input_dates"))
      bNoInputDates = true;
    else if (!strcmp(argv[curr_arg],"-only_max"))
      bOnlyMax = true;
    else if (!strcmp(argv[curr_arg],"-runtot"))
      bRuntot = true;
    else if (!strcmp(argv[curr_arg],"-truncate"))
      bTruncate = true;
    else if (!strcmp(argv[curr_arg],"-insert"))
      bInsert = true;
    else if (!strncmp(argv[curr_arg],"-ge",3))
      sscanf(&argv[curr_arg][3],"%d",&ge_val);
    else if (!strcmp(argv[curr_arg],"-no_distance"))
      bNoDistance = true;
    else if (!strcmp(argv[curr_arg],"-blue_leap"))
      bBlueLeap = true;
    else if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-is_max_blue_distance"))
      bIsMaxBlueDistance = true;
    else if (!strcmp(argv[curr_arg],"-new_year"))
      bNewYear = true;
    else if (!strcmp(argv[curr_arg],"-pct"))
      bPct = true;
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

  if (bBlueLeap)
    bOnlyBlue = true;

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;
  last_blue_line_no = -1;
  bPrevIsBlue = true;
  blue_leap = 0;

  if (bDebug) {
    new_max_count = 0;
    same_max_count = 0;
  }

  balance = initial_bal;
  max_balance = initial_max_bal;

  blue_distance = max_balance - balance;

  if (initial_max_blue_dist)
    max_blue_distance = initial_max_blue_dist;
  else
    max_blue_distance = blue_distance;

  if (bInsert)
    printf("use poker\n\n");

  prev_str[0] = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (!bNoInputDates)
      sscanf(line,"%s\t%d",str,&delta);
    else
      sscanf(line,"%d",&delta);

    balance += delta;

    if (balance > max_balance) {
      if (bBlueLeap)
        blue_leap = balance - max_balance;

      max_balance = balance;
      max_balance_ix = line_no;
    }
    else if (bBlueLeap)
      blue_leap = 0;

    blue_distance = max_balance - balance;

    if (bNewYear) {
      if (!strncmp(prev_str,str,4))
        continue;
      strcpy(prev_str,str);
    }

    if (!bSum) {
      if (!bNoDates) {
        if (!bInSessions) {
          if (!bVerbose) {
            if (!bIsBlue) {
              if (!bSkyfall || ((delta < 0) && (line_no == max_balance_ix + 1))) {
                if (bOnlyBlue && blue_distance)
                  continue;

                if ((ge_val == -1) || (blue_distance >= ge_val)) {
                  if (bTerse)
                    printf("%d\n",blue_distance);
                  else {
                    if (blue_distance >= max_blue_distance) {
                      if (bDebug && (blue_distance == max_blue_distance))
                        same_max_count++;

                      max_blue_distance = blue_distance;

                      if (bDebug)
                        new_max_count++;

                      if (!bRuntot) {
                        if (bTruncate) {
                          if (!bNoDistance)
                            printf("%d\t%s *\n",(bBlueLeap ? blue_leap : blue_distance),str);
                          else
                            printf("%s *\n",str);
                        }
                        else if (bInsert)
                          printf("insert into poker_sessions_blue_distance("
                            "poker_session_date,blue_distance) values ("
                            "'%s',%d);\n",str,blue_distance);
                        else {
                          if (bIsMaxBlueDistance)
                            printf("1 %s\n",line);
                          else if (!bNoDistance)
                            printf("%d\t%s *\n",(bBlueLeap ? blue_leap : blue_distance),line);
                          else
                            printf("%s *\n",line);
                        }
                      }
                      else {
                        if (!bNoDistance)
                          printf("%d\t%d\t%s *\n",(bBlueLeap ? blue_leap : blue_distance),balance,line);
                        else
                          printf("%d\t%s *\n",balance,line);
                      }
                    }
                    else if (!bOnlyMax) {
                      if (!bRuntot) {
                        if (bTruncate) {
                          if (!bNoDistance)
                            printf("%d\t%s\n",(bBlueLeap ? blue_leap : blue_distance),str);
                          else
                            printf("%s\n",str);
                        }
                        else if (bInsert)
                          printf("insert into poker_sessions_blue_distance("
                            "poker_session_date,blue_distance) values ("
                            "'%s',%d);\n",str,blue_distance);
                        else {
                          if (bIsMaxBlueDistance)
                            printf("0 %s\n",line);
                          else if (!bNoDistance)
                            printf("%d\t%s\n",(bBlueLeap ? blue_leap : blue_distance),line);
                          else
                            printf("%s\n",line);
                        }
                      }
                      else {
                        if (!bNoDistance)
                          printf("%d\t%d\t%s\n",(bBlueLeap ? blue_leap : blue_distance),balance,line);
                        else
                          printf("%d\t%s\n",balance,line);
                      }
                    }
                  }
                }
              }
            }
            else {
              printf("%d %d %d\t\%s\n",
                ((max_balance == balance) ? 1 : 0),
                ((max_balance > 0) ? max_balance - balance : max_balance * -1),
                delta,
                line);
            }
          }
          else {
            if (blue_distance > max_blue_distance)
              max_blue_distance = blue_distance;

            if (!bIsBlue) {
              printf("%d (%d %d %d %d)\t%s\n",
                blue_distance,max_blue_distance,
                ((max_balance > 0) ? max_balance : 0),
                balance,
                ((max_balance > 0) ? line_no - max_balance_ix : line_no + 1),
                line);
            }
            else {
              printf("%d (%d %d %d %d) %d %d\t%s\n",
                blue_distance,max_blue_distance,
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

  if (bInsert)
    printf("\nquit\n");

  if (bSum) {
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

  if (bVerbose && (last_blue_line_no != line_no))
    printf("\ncurrent non-blue streak: %d\n",line_no - last_blue_line_no);

  fclose(fptr);

  if (bDebug) {
    printf("new_max_count = %d\n",new_max_count);
    printf("same_max_count = %d\n",same_max_count);
  }

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
