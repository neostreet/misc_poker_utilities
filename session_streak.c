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

static char usage[] = "usage: session_streak (-debug) (-sort) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

struct session_streak_info {
  time_t start_date;
  time_t end_date;
  int datediff;
  int streak;
};

static struct session_streak_info *streak_info;

static char malloc_failed1[] = "malloc of %d session_streak_info structures failed\n";
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

static char *months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
#define NUM_MONTHS (sizeof months / sizeof (char *))

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_session_date(char *line,time_t *session_date);
static time_t cvt_date(char *date_str);
int elem_compare(const void *elem1,const void *elem2);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  int bDebug;
  int bSort;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int session_ix;
  int *ixs;
  int retval;
  int curr_ix;
  int curr_streak;
  int max_streak;
  int max_ix;
  char *cpt;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = FALSE;
  bSort = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
    else if (!strcmp(argv[curr_arg],"-sort"))
      bSort = TRUE;
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

  if ((streak_info = (struct session_streak_info *)malloc(
    num_sessions * sizeof (struct session_streak_info))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 4;
  }

  if ((ixs = (int *)malloc(
    num_sessions * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_sessions);
    fclose(fptr);
    return 5;
  }

  session_ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    retval = get_session_date(line,&streak_info[session_ix].start_date);

    if (retval) {
      printf("get_session_date() failed on line %d: %d\n",session_ix+1,retval);
      return 6;
    }

    session_ix++;
  }

  fclose(fptr);

  for (n = 0; n < num_sessions; n++) {
    if (!n)
      streak_info[n].datediff = 0;
    else
      streak_info[n].datediff = (streak_info[n].start_date - streak_info[n - 1].start_date) /
        (SECS_PER_DAY);
  }

  for (n = 0; n < num_sessions; n++) {
    streak_info[n].streak = -1;
    streak_info[n].end_date = -1;
  }

  for (n = 0; n < num_sessions; n++) {
    for (m = n; m < num_sessions; m++) {
      if (m == n)
        curr_streak = 1;
      else if (streak_info[m].datediff == 1)
        curr_streak++;
      else {
        streak_info[n].end_date = streak_info[m-1].start_date;
        break;
      }
    }

    if ((m == num_sessions) && (streak_info[n].end_date == -1))
      streak_info[n].end_date = streak_info[m-1].start_date;

    streak_info[n].streak = curr_streak;
    n += curr_streak - 1;
  }

  if (bDebug) {
    for (n = 0; n < num_sessions; n++)
      ixs[n] = n;

    if (bSort)
      qsort(ixs,num_sessions,sizeof (int),elem_compare);

    for (n = 0; n < num_sessions; n++) {
      if (streak_info[ixs[n]].streak != -1) {
        cpt = ctime(&streak_info[ixs[n]].start_date);

        printf("%2d %s ",streak_info[ixs[n]].streak,format_date(cpt));

        cpt = ctime(&streak_info[ixs[n]].end_date);

        printf("%s\n",format_date(cpt));
      }
    }

    if (!bSort)
      printf("====================================================\n");
  }

  if (!bDebug || !bSort) {
    max_streak = 0;
    curr_ix = 0;
    curr_streak = 1;

    for (n = 0; n < num_sessions; n++) {
      if (streak_info[n].datediff <= 1)
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

    cpt = ctime(&streak_info[max_ix].start_date);

    printf("%2d %s ",streak_info[max_ix].streak,format_date(cpt));

    cpt = ctime(&streak_info[max_ix].end_date);

    printf("%s\n",format_date(cpt));
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

int elem_compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;

  ix1 = *(int *)elem1;
  ix2 = *(int *)elem2;

  if (streak_info[ix1].streak == streak_info[ix2].streak)
    return streak_info[ix2].start_date - streak_info[ix1].start_date;

  return streak_info[ix2].streak - streak_info[ix1].streak;
}

static char *format_date(char *cpt)
{
  int month;
  static char date_buf[11];

  cpt[7] = 0;
  cpt[10] = 0;
  cpt[24] = 0;

  for (month = 0; month < NUM_MONTHS; month++) {
    if (!strcmp(&cpt[4],months[month]))
      break;
  }

  if (month == NUM_MONTHS)
    month = 0;

  sprintf(date_buf,"%s-%02d-%s",&cpt[20],month+1,&cpt[8]);

  return date_buf;
}
