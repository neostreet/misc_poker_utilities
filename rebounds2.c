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
  time_t poker_session_date;
  int delta;
  int starting_amount;
};

struct rebound_struct {
  time_t poker_session_date;
  int rebound;
};

#define TAB 0x9

static char usage[] = "usage: rebounds2 (-debug) (-no_sort) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char malloc_failed1[] = "malloc of %d session info structures failed\n";
static char malloc_failed2[] = "malloc of %d rebound structures failed\n";
static char malloc_failed3[] = "malloc of %d ints failed\n";

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
static struct rebound_struct *rebound;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_session_info(
  char *line,
  int line_len,
  struct session_info_struct *session_info);
static time_t cvt_date(char *date_str);
int elem_compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int bDebug;
  int bNoSort;
  int session_ix;
  FILE *fptr;
  int line_len;
  int set_size;
  int chara;
  int *sort_ixs;
  int retval;
  char *cpt;
  int num_rebounds;
  int in_losing_streak;
  int rebound_ix;
  int cumulative_loss;
  int cumulative_gain;
  int work;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = FALSE;
  bNoSort = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
    else if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = TRUE;
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

  set_size = 0;

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

    set_size++;
  }

  if ((session_info = (struct session_info_struct *)malloc(
    set_size * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,set_size);
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

    session_ix++;
  }

  fclose(fptr);

  num_rebounds = 0;
  in_losing_streak = 0;

  for (n = 0; n < set_size; n++) {
    if (in_losing_streak) {
      if (session_info[n].delta > 0) {
        num_rebounds++;

        if (bDebug)
          printf("%3d\n",n);

        for (n++; n < set_size; n++) {
          if (session_info[n].delta < 0)
            break;
        }
      }

      continue;
    }
    else {
      if (session_info[n].delta < 0)
        in_losing_streak = 1;

      continue;
    }
  }

  if (bDebug)
    printf("num_rebounds = %d\n",num_rebounds);

  if ((rebound = (struct rebound_struct *)malloc(
    num_rebounds * sizeof (struct rebound_struct))) == NULL) {
    printf(malloc_failed2,num_rebounds);
    fclose(fptr);
    return 6;
  }

  if ((sort_ixs = (int *)malloc(
    num_rebounds * sizeof (int))) == NULL) {
    printf(malloc_failed3,num_rebounds);
    fclose(fptr);
    return 7;
  }

  rebound_ix = 0;
  cumulative_loss = 0;
  cumulative_gain = 0;
  in_losing_streak = 0;

  for (n = 0; n < set_size; n++) {
    if (in_losing_streak) {
      if (session_info[n].delta > 0) {
        cumulative_gain = session_info[n].delta;

        if (bDebug)
          printf("%3d\n",n);

        for (n++; n < set_size; n++) {
          if (session_info[n].delta < 0)
            break;
          else
            cumulative_gain += session_info[n].delta;
        }

        rebound[rebound_ix].poker_session_date =
          session_info[n-1].poker_session_date;

        work = cumulative_loss * -1;

        if (work > cumulative_gain)
          work = cumulative_gain;

        rebound[rebound_ix++].rebound = work;
        cumulative_loss = session_info[n].delta;
      }
      else
        cumulative_loss += session_info[n].delta;

      continue;
    }
    else {
      if (session_info[n].delta < 0) {
        in_losing_streak = 1;
        cumulative_loss = session_info[n].delta;
      }

      continue;
    }
  }

  for (n = 0; n < num_rebounds; n++)
    sort_ixs[n] = n;

  if (!bNoSort)
    qsort(sort_ixs,num_rebounds,sizeof (int),elem_compare);

  for (n = 0; n < num_rebounds; n++) {
    cpt = ctime(&rebound[sort_ixs[n]].poker_session_date);
    cpt[strlen(cpt) - 1] = 0;

    printf("%s %10d\n",
      cpt,rebound[sort_ixs[n]].rebound);
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

  for (n = 0; n < line_len; n++) {
    if (line[n] == TAB)
      break;
  }

  if (n == line_len)
    return 1;

  line[n++] = 0;

  session_info->poker_session_date = cvt_date(line);

  m = n;

  for ( ; n < line_len; n++) {
    if (line[n] == TAB)
      break;
  }

  if (n == line_len)
    return 1;

  line[n++] = 0;

  sscanf(&line[m],"%d",&session_info->delta);
  sscanf(&line[n],"%d",&session_info->starting_amount);

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

  if (rebound[ix2].rebound != rebound[ix1].rebound)
    return rebound[ix2].rebound - rebound[ix1].rebound;

  return rebound[ix2].poker_session_date - rebound[ix1].poker_session_date;
}
