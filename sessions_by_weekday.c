#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define YEAR_IX  0
#define MONTH_IX 1
#define DAY_IX   2

static char *weekdays[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
#define NUM_WEEKDAYS 7

static int weekday_num_sessions[NUM_WEEKDAYS];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define TAB 0x9

static char usage[] =
"usage: sessions_by_weekday (-terse) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

struct digit_range {
  int lower;
  int upper;
};

static struct digit_range date_checks[3] = {
  80, 2095,  /* year */
  1, 12,     /* month */
  1, 31     /* day */
};

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static time_t cvt_date(char *date_str);
static int get_weekday(char *cpt,int *ix);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bTerse;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  time_t poker_session_date;
  char *cpt;
  int ix;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bTerse = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
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
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    poker_session_date = cvt_date(line);

    cpt = ctime(&poker_session_date);

    retval = get_weekday(cpt,&ix);

    if (retval) {
      printf("get_weekday() failed on line %d: %d\n",line_no+1,retval);
      return 4;
    }

    weekday_num_sessions[ix]++;

    line_no++;
  }

  fclose(fptr);

  for (n = 0; n < NUM_WEEKDAYS; n++) {
    if (!bTerse)
      printf("%s: %5d\n",weekdays[n],weekday_num_sessions[n]);
    else
      printf("%d\n",weekday_num_sessions[n]);
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

static time_t cvt_date(char *date_str)
{
  struct tm tim;
  char hold[11];
  int date_len;
  int bufix;
  int holdix;
  int digits[3];
  int n;
  time_t ret_tm;

  date_len = strlen(date_str);

  if (!date_len || (date_len > 10))
    return -1L;

  bufix = 0;

  for (n = 0; n < 3; n++) {
    holdix = 0;

    for ( ; bufix < date_len; ) {
      if (date_str[bufix] == '-') {
        bufix++;
        break;
      }

      if ((date_str[bufix] < '0') || (date_str[bufix] > '9'))
        return -1L;

      hold[holdix++] = date_str[bufix++];
    }

    if (!holdix || ((n != 2) && (bufix == date_len)))
      return -1L;

    hold[holdix] = 0;
    digits[n] = atoi(hold);

    if ((digits[n] > date_checks[n].upper) ||
      (digits[n] < date_checks[n].lower))
      return -1L;
  }

  if (digits[YEAR_IX] >= 100)
    if (digits[YEAR_IX] < 1970)
      return -1L;
    else
      digits[YEAR_IX] -= 1900;

  tim.tm_mon = digits[MONTH_IX] - 1;
  tim.tm_mday = digits[DAY_IX];
  tim.tm_year = digits[YEAR_IX];

  tim.tm_hour = 0;
  tim.tm_min = 0;
  tim.tm_sec = 0;

  tim.tm_isdst = 0;

  ret_tm = mktime(&tim);

  return ret_tm;
}

static int get_weekday(char *cpt,int *ix)
{
  int m;
  int n;

  for (n = 0; n < NUM_WEEKDAYS; n++) {
    for (m = 0; m < 3; m++) {
      if (cpt[m] != weekdays[n][m])
        break;
    }

    if (m == 3) {
      *ix = n;
      return 0;
    }
  }

  return 1;
}
