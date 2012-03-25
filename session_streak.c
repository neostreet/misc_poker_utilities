#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define FALSE 0
#define TRUE  1

#define YEAR_IX  0
#define MONTH_IX 1
#define DAY_IX   2

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: session_streak (-debug) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char malloc_failed1[] = "malloc of %d time_t structures failed\n";
static char malloc_failed2[] = "malloc of %d ints failed\n";

#define SECS_PER_MIN  60
#define MINS_PER_HOUR 60
#define HOURS_PER_DAY 24
#define SECS_PER_DAY (SECS_PER_MIN * MINS_PER_HOUR * HOURS_PER_DAY)

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
static int get_session_date(char *line,time_t *session_date);
static time_t cvt_date(char *date_str);

int main(int argc,char **argv)
{
  int m;
  int n;
  time_t *session_dates;
  int *datediffs;
  int *streaks;
  int curr_arg;
  int bDebug;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int session_ix;
  int retval;
  time_t datediff;
  int curr_ix;
  int curr_streak;
  int max_streak;
  int max_ix;
  char *cpt;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
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

  num_sessions = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    num_sessions++;
  }

  fseek(fptr,0L,SEEK_SET);

  if ((session_dates = (time_t *)malloc(
    num_sessions * sizeof (time_t))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 4;
  }

  if ((datediffs = (int *)malloc(
    num_sessions * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_sessions);
    fclose(fptr);
    return 5;
  }

  if ((streaks = (int *)malloc(
    num_sessions * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_sessions);
    fclose(fptr);
    return 6;
  }

  session_ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    retval = get_session_date(line,&session_dates[session_ix]);

    if (retval) {
      printf("get_session_date() failed on line %d: %d\n",session_ix+1,retval);
      return 7;
    }

    session_ix++;
  }

  fclose(fptr);

  for (n = 0; n < num_sessions; n++) {
    if (!n)
      datediffs[n] = 0;
    else
      datediffs[n] = (session_dates[n] - session_dates[n - 1]) /
      (SECS_PER_DAY);
  }

  for (n = 0; n < num_sessions; n++)
    streaks[n] = -1;

  for (n = 0; n < num_sessions; n++) {
    for (m = n; m < num_sessions; m++) {
      if (m == n)
        curr_streak = 1;
      else if (datediffs[m] == 1)
        curr_streak++;
      else
        break;
    }

    streaks[n] = curr_streak;
    n += curr_streak - 1;
  }

  if (bDebug) {
    for (n = 0; n < num_sessions; n++) {
      if (streaks[n] != -1) {
        cpt = ctime(&session_dates[n]);
        cpt[strlen(cpt)-1] = 0;

        printf("%s %2d\n",cpt,streaks[n]);
      }
    }

    printf("===========================\n");
  }

  max_streak = 0;
  curr_ix = 0;
  curr_streak = 1;

  for (n = 0; n < num_sessions; n++) {
    if (datediffs[n] <= 1)
      curr_streak++;
    else {
      if (curr_streak > max_streak) {
        max_streak = curr_streak;
        max_ix = curr_ix;
      }

      curr_ix = n;
      curr_streak = 1;
    }
  }

  if (curr_streak > max_streak) {
    max_streak = curr_streak;
    max_ix = curr_ix;
  }

  cpt = ctime(&session_dates[max_ix]);
  cpt[strlen(cpt)-1] = 0;
  printf("%2d (%s)\n",max_streak,cpt);

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

static int get_session_date(char *line,time_t *session_date)
{
  *session_date = cvt_date(line);

  if (*session_date == -1L)
    return 1;

  return 0;
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
