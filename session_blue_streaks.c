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

struct session_info_struct {
  time_t start_date;
  time_t end_date;
  int delta;
  int session_is_blue;
  int session_starts_blue_streak;
  int sum;
  int num_blue_sessions;
};

#define TAB 0x9

static char usage[] =
"usage: session_blue_streaks (-no_sort) (-ascending) filename\n";
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
static struct session_info_struct *blue_streaks;
static int bAscending;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_session_info(
  char *line,
  int line_len,
  struct session_info_struct *session_info);
static time_t cvt_date(char *date_str);
int elem_compare(const void *elem1,const void *elem2);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int m;
  int n;
  int bNoSort;
  int curr_arg;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int session_ix;
  int delta;
  int session_is_blue;
  int prev_session_is_blue;
  int chara;
  int *sort_ixs;
  int num_blue_streaks;
  int blue_streak_ix;
  int retval;
  char *cpt;
  int cumulative_delta;
  int max_cumulative_delta;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bNoSort = FALSE;
  bAscending = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = TRUE;
    else if (!strcmp(argv[curr_arg],"-ascending"))
      bAscending = TRUE;
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
  num_blue_streaks = 0;
  cumulative_delta = 0;
  max_cumulative_delta = 0;

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

    cumulative_delta += delta;

    if (cumulative_delta > max_cumulative_delta) {
      session_is_blue = 1;
      max_cumulative_delta = cumulative_delta;
    }
    else
      session_is_blue = 0;

    if ((session_is_blue == 1) && ((num_sessions == 0) || (prev_session_is_blue == 0)))
      num_blue_streaks++;

    num_sessions++;
    prev_session_is_blue = session_is_blue;
  }

  if ((session_info = (struct session_info_struct *)malloc(
    num_sessions * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 4;
  }

  if ((blue_streaks = (struct session_info_struct *)malloc(
    num_blue_streaks * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_blue_streaks);
    fclose(fptr);
    free(session_info);
    return 5;
  }

  if ((sort_ixs = (int *)malloc(
    num_blue_streaks * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_blue_streaks);
    fclose(fptr);
    free(session_info);
    free(blue_streaks);
    return 6;
  }

  for (n = 0; n < num_blue_streaks; n++)
    sort_ixs[n] = n;

  fseek(fptr,0L,SEEK_SET);

  session_ix = 0;
  cumulative_delta = 0;
  max_cumulative_delta = 0;

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
      free(blue_streaks);
      free(sort_ixs);
      return 7;
    }

    cumulative_delta += session_info[session_ix].delta;

    if (cumulative_delta > max_cumulative_delta) {
      session_info[session_ix].session_is_blue = 1;
      max_cumulative_delta = cumulative_delta;
    }
    else
      session_info[session_ix].session_is_blue = 0;

    if ((session_info[session_ix].session_is_blue == 1) &&
        ((session_ix == 0) || (session_info[session_ix-1].session_is_blue == 0)))
      session_info[session_ix].session_starts_blue_streak = 1;
    else
      session_info[session_ix].session_starts_blue_streak = 0;

    session_ix++;
  }

  fclose(fptr);

  blue_streak_ix = 0;

  for (n = 0; n < num_sessions; n++) {
    if (session_info[n].session_starts_blue_streak) {
      blue_streaks[blue_streak_ix].start_date = session_info[n].start_date;

      blue_streaks[blue_streak_ix].sum = session_info[n].delta;

      for (m = n + 1; (session_info[m].session_is_blue == 1); m++)
        blue_streaks[blue_streak_ix].sum += session_info[m].delta;

      blue_streaks[blue_streak_ix].end_date = session_info[m - 1].start_date;
      blue_streaks[blue_streak_ix].num_blue_sessions = m - n;

      blue_streak_ix++;
    }
  }

  if (!bNoSort)
    qsort(sort_ixs,num_blue_streaks,sizeof (int),elem_compare);

  for (n = 0; n < num_blue_streaks; n++) {
    printf("%3d ",blue_streaks[sort_ixs[n]].num_blue_sessions);

    cpt = ctime(&blue_streaks[sort_ixs[n]].start_date);
    printf("%s ",format_date(cpt));

    cpt = ctime(&blue_streaks[sort_ixs[n]].end_date);
    printf("%s ",format_date(cpt));

    printf("%10d\n",blue_streaks[sort_ixs[n]].sum);
  }

  free(session_info);
  free(blue_streaks);
  free(sort_ixs);

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

int elem_compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;

  ix1 = *(int *)elem1;
  ix2 = *(int *)elem2;

  if (blue_streaks[ix1].num_blue_sessions == blue_streaks[ix2].num_blue_sessions)
    return blue_streaks[ix2].end_date - blue_streaks[ix1].end_date;

  if (bAscending)
    return blue_streaks[ix1].num_blue_sessions - blue_streaks[ix2].num_blue_sessions;
  else
    return blue_streaks[ix2].num_blue_sessions - blue_streaks[ix1].num_blue_sessions;
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
