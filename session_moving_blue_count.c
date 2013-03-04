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
  int is_blue;
  int blue_count;
  int num_winning_sessions;
};

#define TAB 0x9

static char usage[] =
"usage: session_moving_blue_count (-no_sort) (-ascending) subset_size filename\n";
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
static bool bAscending;

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
  bool bNoSort;
  int curr_arg;
  int session_ix;
  int subset_size;
  FILE *fptr;
  int line_len;
  int set_size;
  int sum_delta;
  int max_sum_delta;
  int chara;
  int *sort_ixs;
  int num_blue_counts;
  int blue_count;
  int num_winning_sessions;
  int retval;
  char *cpt;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bNoSort = false;
  bAscending = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = true;
    else if (!strcmp(argv[curr_arg],"-ascending"))
      bAscending = true;
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

  num_blue_counts = set_size - subset_size + 1;

  if ((session_info = (struct session_info_struct *)malloc(
    set_size * sizeof (struct session_info_struct))) == NULL) {
    printf(malloc_failed1,set_size);
    fclose(fptr);
    return 5;
  }

  if ((sort_ixs = (int *)malloc(
    num_blue_counts * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_blue_counts);
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

    retval = get_session_info(line,line_len,&session_info[session_ix]);

    if (retval) {
      printf("get_session_info() failed on line %d: %d\n",
        session_ix+1,retval);
      return 7;
    }

    session_ix++;
  }

  sum_delta = 0;
  max_sum_delta = 0;

  for (n = 0; n < set_size; n++) {
    sum_delta += session_info[n].delta;

    if (sum_delta > max_sum_delta) {
      session_info[n].is_blue = 1;
      max_sum_delta = sum_delta;
    }
    else
      session_info[n].is_blue = 0;
  }

  for (n = 0; n < num_blue_counts; n++) {
    blue_count = 0;
    num_winning_sessions = 0;

    for (m = 0; m < subset_size; m++) {
      blue_count += session_info[n+m].is_blue;

      if (session_info[n+m].delta > 0)
        num_winning_sessions++;
    }

    session_info[n].blue_count = blue_count;
    session_info[n].num_winning_sessions = num_winning_sessions;
    session_info[n].end_date = session_info[n+subset_size-1].start_date;
    sort_ixs[n] = n;
  }

  if (!bNoSort)
    qsort(sort_ixs,num_blue_counts,sizeof (int),elem_compare);

  for (n = 0; n < num_blue_counts; n++) {
    printf("%10d ",session_info[sort_ixs[n]].blue_count);

    cpt = ctime(&session_info[sort_ixs[n]].start_date);
    printf("%s ",format_date(cpt));

    cpt = ctime(&session_info[sort_ixs[n]].end_date);
    printf("%s ",format_date(cpt));

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

  if (session_info[ix1].blue_count == session_info[ix2].blue_count)
    return session_info[ix2].end_date - session_info[ix1].end_date;

  if (bAscending)
    return session_info[ix1].blue_count - session_info[ix2].blue_count;
  else
    return session_info[ix2].blue_count - session_info[ix1].blue_count;
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
