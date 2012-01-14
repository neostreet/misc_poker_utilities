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
  int sum;
  int nobs;
};

#define TAB 0x9

static char usage[] =
"usage: condense (-debug) (-sort) (-ascending) filename\n";
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

static struct session_info_struct *session_info;
static int bAscending;

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
  int bDebug;
  int bSort;
  int curr_arg;
  int session_ix;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int chara;
  int *sort_ixs;
  int retval;
  char *cpt;
  int num_condensed_sessions;
  int condensed_ix;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = FALSE;
  bSort = FALSE;
  bAscending = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
    else if (!strcmp(argv[curr_arg],"-sort"))
      bSort = TRUE;
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

    num_sessions++;
  }

  if ((session_info = (struct session_info_struct *)malloc(
    num_sessions * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 4;
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
      return 5;
    }

    session_info[session_ix].nobs = -1;

    session_ix++;
  }

  fclose(fptr);

  if (bDebug) {
    for (n = 0; n < num_sessions; n++) {
      cpt = ctime(&session_info[n].start_date);
      cpt[strlen(cpt) - 1] = 0;
      printf("%s %10d\n",cpt,session_info[n].delta);
    }
  }

  n = 0;
  num_condensed_sessions = 0;

  for ( ; ; ) {
    session_info[n].sum = session_info[n].delta;

    for (m = n + 1; m < num_sessions; m++) {
      if (session_info[n].delta < 0) {
        if (session_info[m].delta < 0)
          session_info[n].sum += session_info[m].delta;
        else
          break;
      }
      else {
        if (session_info[m].delta > 0)
          session_info[n].sum += session_info[m].delta;
        else
          break;
      }
    }

    num_condensed_sessions++;
    session_info[n].nobs = m - n;
    session_info[n].end_date = session_info[m - 1].start_date;

    if (m == num_sessions)
      break;

    n = m;
  }

  condensed_ix = 0;

  for (n = 0; n < num_sessions; n++) {
    if (session_info[n].nobs != -1) {
      if (condensed_ix != n)
        session_info[condensed_ix] = session_info[n];

      condensed_ix++;
    }
  }

  if ((sort_ixs = (int *)malloc(
    num_condensed_sessions * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_condensed_sessions);
    fclose(fptr);
    return 6;
  }

  for (n = 0; n < num_condensed_sessions; n++)
    sort_ixs[n] = n;

  if (bSort)
    qsort(sort_ixs,num_condensed_sessions,sizeof (int),elem_compare);

  for (n = 0; n < num_condensed_sessions; n++) {
    printf("%10d ",session_info[sort_ixs[n]].sum);

    cpt = ctime(&session_info[sort_ixs[n]].start_date);
    cpt[strlen(cpt) - 1] = 0;
    printf("%s ",cpt);

    cpt = ctime(&session_info[sort_ixs[n]].end_date);
    cpt[strlen(cpt) - 1] = 0;
    printf("%s (%3d)\n",cpt,session_info[sort_ixs[n]].nobs);
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

  if (session_info[ix1].sum == session_info[ix2].sum)
    return session_info[ix2].end_date - session_info[ix1].end_date;

  if (bAscending)
    return session_info[ix1].sum - session_info[ix2].sum;
  else
    return session_info[ix2].sum - session_info[ix1].sum;
}
