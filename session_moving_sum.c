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
  int num_winning_sessions;
};

#define TAB 0x9

static char usage[] =
"usage: session_moving_sum (-no_sort) (-ascending) (-absolute_value) subset_size filename\n";
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
  struct session_info_struct *session_info,
  int bAbsoluteValue);
static time_t cvt_date(char *date_str);
int elem_compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int m;
  int n;
  int bNoSort;
  int bAbsoluteValue;
  int curr_arg;
  int session_ix;
  int subset_size;
  FILE *fptr;
  int line_len;
  int set_size;
  int chara;
  int *sort_ixs;
  int num_sums;
  int sum;
  int num_winning_sessions;
  int retval;
  char *cpt;

  if ((argc < 3) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bNoSort = FALSE;
  bAscending = FALSE;
  bAbsoluteValue = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = TRUE;
    else if (!strcmp(argv[curr_arg],"-ascending"))
      bAscending = TRUE;
    else if (!strcmp(argv[curr_arg],"-absolute_value"))
      bAbsoluteValue = TRUE;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&subset_size);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
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

  if (subset_size > set_size) {
    printf("subset_size (%d) > set_size (%d)\n",subset_size,set_size);
    fclose(fptr);
    return 4;
  }

  num_sums = set_size - subset_size + 1;

  if ((session_info = (struct session_info_struct *)malloc(
    set_size * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,set_size);
    fclose(fptr);
    return 5;
  }

  if ((sort_ixs = (int *)malloc(
    num_sums * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_sums);
    fclose(fptr);
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

    retval = get_session_info(line,line_len,&session_info[session_ix],
      bAbsoluteValue);

    if (retval) {
      printf("get_session_info() failed on line %d: %d\n",
        session_ix+1,retval);
      return 7;
    }

    session_ix++;
  }

  for (n = 0; n < num_sums; n++) {
    sum = 0;
    num_winning_sessions = 0;

    for (m = 0; m < subset_size; m++) {
      sum += session_info[n+m].delta;

      if (session_info[n+m].delta > 0)
        num_winning_sessions++;
    }

    session_info[n].sum = sum;
    session_info[n].num_winning_sessions = num_winning_sessions;
    session_info[n].end_date = session_info[n+subset_size-1].start_date;
    sort_ixs[n] = n;
  }

  if (!bNoSort)
    qsort(sort_ixs,num_sums,sizeof (int),elem_compare);

  for (n = 0; n < num_sums; n++) {
    printf("%10d ",session_info[sort_ixs[n]].sum);

    cpt = ctime(&session_info[sort_ixs[n]].start_date);
    cpt[strlen(cpt) - 1] = 0;
    printf("%s ",cpt);

    cpt = ctime(&session_info[sort_ixs[n]].end_date);
    cpt[strlen(cpt) - 1] = 0;
    printf("%s ",cpt);

    printf("(%d)\n",session_info[sort_ixs[n]].num_winning_sessions);
  }

  fclose(fptr);
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
  struct session_info_struct *session_info,
  int bAbsoluteValue)
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

  if ((bAbsoluteValue) && (work < 0))
    work *= -1;

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
