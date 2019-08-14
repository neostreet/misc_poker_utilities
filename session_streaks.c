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

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

struct session_info_struct {
  time_t start_date;
  time_t end_date;
  int delta;
  int sum;
  int num_sessions;
};

#define TAB 0x9

static char usage[] =
"usage: session_streaks (-ascending) (-sort_by_sum_delta) (-total)\n"
"  (-only_winning) (-only_losing) (-ge_val) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char malloc_failed1[] = "malloc of %d session info structures failed\n";
static char malloc_failed2[] = "malloc of %d ints failed\n";

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

static struct session_info_struct *session_info;
static struct session_info_struct *streaks;
static bool bAscending;
static bool bSortBySumDelta;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_session_info(
  char *line,
  int line_len,
  struct session_info_struct *session_info);
static time_t cvt_date(char *date_str);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int m;
  int n;
  bool bTotal;
  int curr_arg;
  bool bOnlyWinning;
  bool bOnlyLosing;
  int ge_val;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int session_ix;
  int delta;
  int prev_delta;
  int chara;
  int num_streaks;
  int streak_ix;
  int retval;
  char *cpt;
  int total_sessions;
  int total_session_deltas;

  if ((argc < 2) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bTotal = false;
  bAscending = false;
  bSortBySumDelta = false;

  bOnlyWinning = false;
  bOnlyLosing = false;
  ge_val = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-ascending"))
      bAscending = true;
    else if (!strcmp(argv[curr_arg],"-sort_by_sum_delta"))
      bSortBySumDelta = true;
    else if (!strcmp(argv[curr_arg],"-total"))
      bTotal = true;
    else if (!strcmp(argv[curr_arg],"-only_winning"))
      bOnlyWinning = true;
    else if (!strcmp(argv[curr_arg],"-only_losing"))
      bOnlyLosing = true;
    else if (!strncmp(argv[curr_arg],"-ge_",4))
      sscanf(&argv[curr_arg][4],"%d",&ge_val);
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bOnlyWinning && bOnlyLosing) {
    printf("can't specify both -only_winning and -only_losing\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  num_sessions = 0;
  num_streaks = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (!line_len)
      continue;

    chara = line[0];

    if (((chara >= 'a') && (chara <= 'z')) ||
        ((chara >= 'A') && (chara <= 'Z')))
      continue;

    sscanf(&line[11],"%d",&delta);

    if ((num_sessions == 0) || ((delta > 0) && (prev_delta < 0)) ||
        ((delta < 0) && (prev_delta > 0)))
      num_streaks++;

    num_sessions++;
    prev_delta = delta;
  }

  if ((session_info = (struct session_info_struct *)malloc(
    num_sessions * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 5;
  }

  if ((streaks = (struct session_info_struct *)malloc(
    num_streaks * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_streaks);
    fclose(fptr);
    free(session_info);
    return 6;
  }

  fseek(fptr,0L,SEEK_SET);

  session_ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (!line_len)
      continue;

    chara = line[0];

    if (((chara >= 'a') && (chara <= 'z')) ||
        ((chara >= 'A') && (chara <= 'Z')))
      continue;

    retval = get_session_info(line,line_len,&session_info[session_ix]);

    if (retval) {
      printf("get_session_info() failed on line %d: %d\n",
        session_ix+1,retval);
      fclose(fptr);
      free(session_info);
      free(streaks);
      return 7;
    }

    session_ix++;
  }

  fclose(fptr);

  streak_ix = 0;

  for (n = 0; n < num_sessions; n++) {
    if ((n == 0) || ((session_info[n].delta > 0) && (session_info[n-1].delta < 0)) ||
        ((session_info[n].delta < 0) && (session_info[n-1].delta > 0))) {
      streaks[streak_ix].start_date = session_info[n].start_date;

      streaks[streak_ix].sum = session_info[n].delta;

      for (m = n + 1; (m < num_sessions) && (((session_info[n].delta > 0) && (session_info[m].delta > 0)) ||
        ((session_info[n].delta < 0) && (session_info[m].delta < 0))); m++)
        streaks[streak_ix].sum += session_info[m].delta;

      streaks[streak_ix].end_date = session_info[m - 1].start_date;
      streaks[streak_ix].num_sessions = m - n;

      streak_ix++;
    }
  }

  if (bTotal) {
    total_sessions = 0;
    total_session_deltas = 0;
  }

  for (n = 0; n < num_streaks; n++) {
    if (bOnlyWinning && streaks[n].sum < 0)
      continue;

    if (bOnlyLosing && streaks[n].sum > 0)
      continue;

    if (streaks[n].num_sessions < ge_val)
      continue;

    printf("%3d ",streaks[n].num_sessions);

    cpt = ctime(&streaks[n].start_date);
    printf("%s ",format_date(cpt));

    cpt = ctime(&streaks[n].end_date);
    printf("%s ",format_date(cpt));

    printf("%10d\n",streaks[n].sum);

    if (bTotal) {
      total_sessions += streaks[n].num_sessions;
      total_session_deltas += streaks[n].sum;
    }
  }

  free(session_info);
  free(streaks);

  if (bTotal) {
    printf("\n%3d                       %10d\n",
      total_sessions,total_session_deltas);
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

static int get_session_info(
  char *line,
  int line_len,
  struct session_info_struct *session_info)
{
  int n;
  int work;

  for (n = 0; n < line_len; n++) {
    if (line[n] == TAB)
      break;
  }

  if (n == line_len)
    return 1;

  line[n++] = 0;

  session_info->start_date = cvt_date(line);

  sscanf(&line[n],"%d",&work);

  session_info->delta = work;

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

  if (cpt[8] == ' ')
    cpt[8] = '0';

  sprintf(date_buf,"%s-%02d-%s",&cpt[20],month+1,&cpt[8]);

  return date_buf;
}
