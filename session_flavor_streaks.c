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
  int sit_and_go;
  int flavor;
  int num_flavor_sessions;
};

#define TAB 0x9

static char usage[] =
"usage: session_flavor_streaks (-debug) (-no_sort) (-ascending) (-sit_and_go) filename\n";
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
static struct session_info_struct *flavor_streaks;
static bool bAscending;
static bool bSitAndGo;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_session_info(
  char *line,
  int line_len,
  struct session_info_struct *session_info,
  bool bSitAndGo);
static time_t cvt_date(char *date_str);
int elem_compare(const void *elem1,const void *elem2);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int m;
  int n;
  bool bDebug;
  bool bNoSort;
  int curr_arg;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int session_ix;
  int *sort_ixs;
  int num_flavor_streaks;
  int flavor_streak_ix;
  int retval;
  char *cpt;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bNoSort = false;
  bAscending = false;
  bSitAndGo = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = true;
    else if (!strcmp(argv[curr_arg],"-ascending"))
      bAscending = true;
    else if (!strcmp(argv[curr_arg],"-sit_and_go"))
      bSitAndGo = true;
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

  if ((session_info = (struct session_info_struct *)malloc(
    num_sessions * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 4;
  }

  if ((flavor_streaks = (struct session_info_struct *)malloc(
    num_sessions * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    free(session_info);
    return 5;
  }

  if ((sort_ixs = (int *)malloc(
    num_sessions * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_sessions);
    fclose(fptr);
    free(session_info);
    free(flavor_streaks);
    return 6;
  }

  fseek(fptr,0L,SEEK_SET);

  session_ix = 0;
  num_flavor_streaks = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    retval = get_session_info(line,line_len,&session_info[session_ix],
      bSitAndGo);

    if (retval) {
      printf("get_session_info() failed on line %d: %d\n",
        session_ix+1,retval);
      fclose(fptr);
      free(session_info);
      free(flavor_streaks);
      free(sort_ixs);
      return 7;
    }

    if ((session_ix == 0) ||
        (session_info[session_ix].flavor != session_info[session_ix-1].flavor) ||
        (bSitAndGo &&
        (session_info[session_ix].sit_and_go != session_info[session_ix-1].sit_and_go))) {

      sort_ixs[num_flavor_streaks] = num_flavor_streaks;
      num_flavor_streaks++;
    }

    if (bDebug) {
      if (!bSitAndGo)
        printf("%d\n",session_info[session_ix].flavor);
      else {
        printf("%d %d\n",session_info[session_ix].flavor,
          session_info[session_ix].sit_and_go);
      }
    }

    session_ix++;
  }

  fclose(fptr);

  flavor_streak_ix = 0;

  for (n = 0; n < num_sessions; n++) {
    if ((n == 0) ||
        (session_info[n].flavor != session_info[n-1].flavor) ||
        (bSitAndGo &&
        (session_info[n].sit_and_go != session_info[n-1].sit_and_go))) {

      flavor_streaks[flavor_streak_ix].start_date = session_info[n].start_date;
      flavor_streaks[flavor_streak_ix].sum = session_info[n].delta;

      for (m = n + 1;
          (m < num_sessions) &&
          (!bSitAndGo || (session_info[m].sit_and_go == session_info[n].sit_and_go)) &&
          (session_info[m].flavor == session_info[n].flavor);
          m++)
        flavor_streaks[flavor_streak_ix].sum += session_info[m].delta;

      flavor_streaks[flavor_streak_ix].end_date = session_info[m - 1].start_date;
      flavor_streaks[flavor_streak_ix].num_flavor_sessions = m - n;
      flavor_streaks[flavor_streak_ix].sit_and_go = session_info[n].sit_and_go;
      flavor_streaks[flavor_streak_ix].flavor = session_info[n].flavor;

      flavor_streak_ix++;
    }
  }

  if (!bNoSort)
    qsort(sort_ixs,num_flavor_streaks,sizeof (int),elem_compare);

  for (n = 0; n < num_flavor_streaks; n++) {
    printf("%3d ",flavor_streaks[sort_ixs[n]].num_flavor_sessions);

    cpt = ctime(&flavor_streaks[sort_ixs[n]].start_date);
    printf("%s ",format_date(cpt));

    cpt = ctime(&flavor_streaks[sort_ixs[n]].end_date);
    printf("%s ",format_date(cpt));

    if (!bSitAndGo) {
      printf("%d %10d\n",flavor_streaks[sort_ixs[n]].flavor,
        flavor_streaks[sort_ixs[n]].sum);
    }
    else {
      printf("%d %d %10d\n",
        flavor_streaks[sort_ixs[n]].sit_and_go,
        flavor_streaks[sort_ixs[n]].flavor,
        flavor_streaks[sort_ixs[n]].sum);
    }
  }

  free(session_info);
  free(flavor_streaks);
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
  struct session_info_struct *session_info,
  bool bSitAndGo)
{
  int m;
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

  m = n;

  for ( ; n < line_len; n++) {
    if (line[n] == TAB)
      break;
  }

  if (n == line_len)
    return 1;

  line[n++] = 0;

  sscanf(&line[m],"%d",&work);
  session_info->delta = work;

  if (!bSitAndGo) {
    sscanf(&line[n],"%d",&work);
    session_info->flavor = work;
  }
  else {
    m = n;

    for ( ; n < line_len; n++) {
      if (line[n] == TAB)
        break;
    }

    if (n == line_len)
      return 1;

    line[n++] = 0;

    sscanf(&line[m],"%d",&work);
    session_info->sit_and_go = work;
    sscanf(&line[n],"%d",&work);
    session_info->flavor = work;
  }

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

  if (flavor_streaks[ix1].num_flavor_sessions == flavor_streaks[ix2].num_flavor_sessions)
    return flavor_streaks[ix2].end_date - flavor_streaks[ix1].end_date;

  if (bAscending)
    return flavor_streaks[ix1].num_flavor_sessions - flavor_streaks[ix2].num_flavor_sessions;
  else
    return flavor_streaks[ix2].num_flavor_sessions - flavor_streaks[ix1].num_flavor_sessions;
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
