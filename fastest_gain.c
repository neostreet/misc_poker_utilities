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

#define TAB 0x9

static char usage[] = "usage: fastest_gain (-verbose) (-reverse) amount filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static int bReverse;

static char malloc_failed1[] = "malloc of %d session info structures failed\n";
static char malloc_failed2[] = "malloc of %d ints failed\n";

static char fmt1[] = "%10d %4d ";
static char fmt2[] = "%10d %4d %10.2lf (%d)\n";

struct digit_range {
  int lower;
  int upper;
};

static struct digit_range date_checks[3] = {
  80, 2095,  /* year */
  1, 12,     /* month */
  1, 31     /* day */
};

struct session_info_struct {
  int starting_amount;
  int starting_ix;
  int ending_amount;
  int gain_amount;
  int num_gain_sessions;
  int winning_session;
  int num_winning_sessions;
  time_t gain_start_date;
  time_t gain_end_date;
};

static struct session_info_struct *session_info;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_session_info(
  char *line,
  int line_len,
  struct session_info_struct *session_info);
static time_t cvt_date(char *date_str);
int elem_compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  int bVerbose;
  int gain_threshold;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int num_winning_sessions;
  int num_gains;
  int *sort_ixs;
  int ix;
  int retval;
  char *cpt;
  double avg_amount;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = FALSE;
  bReverse = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = TRUE;
    else if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = TRUE;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&gain_threshold);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
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

  if ((session_info = (struct session_info_struct *)malloc(
    num_sessions * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 4;
  }

  ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    retval = get_session_info(line,line_len,&session_info[ix]);

    session_info[ix].num_gain_sessions = -1;
    session_info[ix].starting_ix = ix;

    ix++;
  }

  fclose(fptr);

  for (m = 0; m < num_sessions; m++) {
    for (n = m; n < num_sessions; n++) {
      if (session_info[n].ending_amount - session_info[m].starting_amount
        >= gain_threshold) {

        num_winning_sessions = 0;

        for (p = m; p <= n; p++)
          num_winning_sessions += session_info[p].winning_session;

        session_info[m].num_gain_sessions = n - m + 1;
        session_info[m].num_winning_sessions = num_winning_sessions;
        session_info[m].gain_amount =
          session_info[n].ending_amount - session_info[m].starting_amount;
        session_info[m].gain_end_date = session_info[n].gain_start_date;

        break;
      }
    }
  }

  num_gains = 0;

  for (n = 0; n < num_sessions; n++) {
    if (session_info[n].num_gain_sessions != -1) {
      if (num_gains != n)
        session_info[num_gains] = session_info[n];

      num_gains++;
    }
  }

  if ((sort_ixs = (int *)malloc(
    num_gains * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_gains);
    fclose(fptr);
    return 5;
  }

  for (n = 0; n < num_gains; n++)
    sort_ixs[n] = n;

  qsort(sort_ixs,num_gains,sizeof (int),elem_compare);

  for (n = 0; n < num_gains; n++) {
    printf(fmt1,
      session_info[sort_ixs[n]].starting_amount,
      session_info[sort_ixs[n]].starting_ix);

    cpt = ctime(&session_info[sort_ixs[n]].gain_start_date);
    cpt[strlen(cpt) - 1] = 0;
    printf("%s\n",cpt);

    printf(fmt1,
      session_info[sort_ixs[n]].starting_amount +
        session_info[sort_ixs[n]].gain_amount,
      session_info[sort_ixs[n]].starting_ix +
        session_info[sort_ixs[n]].num_gain_sessions - 1);

    cpt = ctime(&session_info[sort_ixs[n]].gain_end_date);
    cpt[strlen(cpt) - 1] = 0;
    printf("%s\n",cpt);

    avg_amount = (double)session_info[sort_ixs[n]].gain_amount /
      (double)session_info[sort_ixs[n]].num_gain_sessions;

    printf(fmt2,
      session_info[sort_ixs[n]].gain_amount,
      session_info[sort_ixs[n]].num_gain_sessions,
      avg_amount,
      session_info[sort_ixs[n]].num_winning_sessions);

    if (!bVerbose)
      break;

    if (n < num_gains - 1)
      putchar(0x0a);
  }

  free(session_info);
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

  session_info->gain_start_date = cvt_date(line);

  for (m = n; n < line_len; n++) {
    if (line[n] == TAB)
      break;
  }

  if (n == line_len)
    return 2;

  line[n++] = 0;

  sscanf(&line[m],"%d",&work);

  session_info->starting_amount = work;

  sscanf(&line[n],"%d",&work);

  session_info->ending_amount = work;

  if (session_info->ending_amount > session_info->starting_amount)
    session_info->winning_session = 1;
  else
    session_info->winning_session = 0;

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

  if (!bReverse) {
    if (session_info[ix1].num_gain_sessions !=
        session_info[ix2].num_gain_sessions) {
      return session_info[ix1].num_gain_sessions -
        session_info[ix2].num_gain_sessions;
    }
    else if (session_info[ix1].gain_amount !=
        session_info[ix2].gain_amount) {
      return session_info[ix2].gain_amount -
        session_info[ix1].gain_amount;
    }
    else {
      return session_info[ix2].gain_end_date -
        session_info[ix1].gain_end_date;
    }
  }
  else {
    if (session_info[ix1].num_gain_sessions !=
        session_info[ix2].num_gain_sessions) {
      return session_info[ix2].num_gain_sessions -
        session_info[ix1].num_gain_sessions;
    }
    else if (session_info[ix1].gain_amount !=
        session_info[ix2].gain_amount) {
      return session_info[ix1].gain_amount -
        session_info[ix2].gain_amount;
    }
    else {
      return session_info[ix2].gain_end_date -
        session_info[ix1].gain_end_date;
    }
  }
}
