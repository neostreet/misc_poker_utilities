#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_MILLION_BOUNDARIES 500
static int crossings[MAX_MILLION_BOUNDARIES];

#define FIRST_YEAR 2009

static char usage[] =
"usage: million_boundary_crossings (-verbose) (-up) (-down) (-by_year) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bVerbose;
  bool bUp;
  bool bDown;
  bool bEither;
  bool bByYear;
  FILE *fptr;
  int line_len;
  int line_no;
  int year;
  int month;
  int day;
  int adjusted_year;
  int max_adjusted_year;
  int starting_balance;
  int ending_balance;
  int starting_million;
  int ending_million;
  int dbg_million;
  int dbg;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  dbg_million = 0;

  bVerbose = false;
  bUp = false;
  bDown = false;
  bEither = true;
  bByYear = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-up"))
      bUp = true;
    else if (!strcmp(argv[curr_arg],"-down"))
      bDown = true;
    else if (!strcmp(argv[curr_arg],"-by_year"))
      bByYear = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bUp && bDown) {
    printf("can't specify both -up and -down\n");
    return 3;
  }

  if (bUp || bDown)
    bEither = false;

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  for (n = 0; n < MAX_MILLION_BOUNDARIES; n++)
    crossings[n] = 0;

  line_no = 0;

  if (bByYear)
    max_adjusted_year = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d-%d-%d\t%d\t%d",&year,&month,&day,&starting_balance,&ending_balance);

    starting_million = starting_balance / 1000000;
    ending_million = ending_balance / 1000000;

    if (ending_million >= MAX_MILLION_BOUNDARIES) {
      printf("ending_million (%d) > MAX_MILLION_BOUNDARIES (%d)\n",
        ending_million,MAX_MILLION_BOUNDARIES);
      fclose(fptr);
      return 5;
    }

    if (starting_million == ending_million)
      continue;

    if (((starting_million < dbg_million) && (ending_million >= dbg_million)) ||
        ((starting_million >= dbg_million) && (ending_million < dbg_million)))
      dbg = 1;

    if (starting_million < ending_million) {
      if (bEither || bUp) {
        for (starting_million++; starting_million <= ending_million; starting_million++) {
          if (!bVerbose) {
            if (!bByYear)
              crossings[starting_million]++;
            else {
              adjusted_year = year - FIRST_YEAR;

              if ((adjusted_year < 0) || (adjusted_year >= MAX_MILLION_BOUNDARIES)) {
                printf("year out of bounds on line %d\n",line_no);
                return 6;
              }

              crossings[adjusted_year]++;

              if (adjusted_year > max_adjusted_year)
                max_adjusted_year = adjusted_year;
            }
          }
          else
            printf("%d-%02d-%02d %d\n",year,month,day,starting_million);
        }
      }
    }
    else {
      if (bEither || bDown) {
        for (starting_million--; starting_million >= ending_million; starting_million--) {
          if (!bVerbose) {
            if (!bByYear)
              crossings[starting_million+1]++;
            else {
              adjusted_year = year - FIRST_YEAR;

              if ((adjusted_year < 0) || (adjusted_year >= MAX_MILLION_BOUNDARIES)) {
                printf("year out of bounds on line %d\n",line_no);
                return 6;
              }

              crossings[adjusted_year]++;

              if (adjusted_year > max_adjusted_year)
                max_adjusted_year = adjusted_year;
            }
          }
          else
            printf("%d-%02d-%02d %d\n",year,month,day,starting_million);
        }
      }
    }
  }

  fclose(fptr);

  if (!bVerbose) {
    if (!bByYear) {
      for (n = MAX_MILLION_BOUNDARIES - 1; (n > 0); n--) {
        if (crossings[n])
          break;
      }

      for (m = 1; m <= n; m++)
        printf("%2d %8d\n",crossings[m],m * 1000000);
    }
    else {
      for (m = 0; m <= max_adjusted_year; m++)
        printf("%4d %d\n",crossings[m],m + FIRST_YEAR);
    }
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
