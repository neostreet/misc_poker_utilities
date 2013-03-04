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

#define TAB 0x9

static char usage[] = "usage: max_gain (-debug) (-verbose) (-no_sort) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char malloc_failed1[] = "malloc of %d session info structures failed\n";
static char malloc_failed2[] = "malloc of %d ints failed\n";

static char fmt1[] = "%10d %4d ";
static char fmt2[] = "%10d %4d\n";

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

struct session_info_struct {
  int starting_amount;
  int starting_ix;
  int ending_amount;
  int gain_amount;
  int num_gain_sessions;
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
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bNoSort;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int max_gain;
  int work;
  int max_gain_ix;
  int num_gains;
  int *sort_ixs;
  int ix;
  int retval;
  char *cpt;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bNoSort = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = true;
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
    max_gain = 0;

    for (n = m; n < num_sessions; n++) {
      work = session_info[n].ending_amount - session_info[m].starting_amount;

      if (work > max_gain) {
        max_gain = work;
        max_gain_ix = n;
      }
    }

    if (max_gain) {
      session_info[m].num_gain_sessions = max_gain_ix - m + 1;
      session_info[m].gain_amount = max_gain;
      session_info[m].gain_end_date = session_info[max_gain_ix].gain_start_date;
    }
  }

  num_gains = 0;

  for (n = 0; n < num_sessions; n++) {
    if (session_info[n].num_gain_sessions != -1) {
      if (num_gains != n)
        session_info[num_gains] = session_info[n];

      if (bDebug) {
        printf(fmt1,
          session_info[n].starting_amount,
          session_info[n].starting_ix);

        cpt = ctime(&session_info[n].gain_start_date);
        printf("%s\n",format_date(cpt));

        printf(fmt1,
          session_info[n].starting_amount +
            session_info[n].gain_amount,
          session_info[n].starting_ix +
            session_info[n].num_gain_sessions - 1);

        cpt = ctime(&session_info[n].gain_end_date);
        printf("%s\n",format_date(cpt));

        printf(fmt2,
          session_info[n].gain_amount,
          session_info[n].num_gain_sessions);
      }

      num_gains++;
    }
  }

  if (bDebug)
    printf("**********************************************************\n");

  if ((sort_ixs = (int *)malloc(
    num_gains * sizeof (int))) == NULL) {
    printf(malloc_failed2,num_gains);
    fclose(fptr);
    return 5;
  }

  for (n = 0; n < num_gains; n++)
    sort_ixs[n] = n;

  if (!bNoSort)
    qsort(sort_ixs,num_gains,sizeof (int),elem_compare);

  for (n = 0; n < num_gains; n++) {
    printf(fmt1,
      session_info[sort_ixs[n]].starting_amount,
      session_info[sort_ixs[n]].starting_ix);

    cpt = ctime(&session_info[sort_ixs[n]].gain_start_date);
    printf("%s\n",format_date(cpt));

    printf(fmt1,
      session_info[sort_ixs[n]].starting_amount +
        session_info[sort_ixs[n]].gain_amount,
      session_info[sort_ixs[n]].starting_ix +
        session_info[sort_ixs[n]].num_gain_sessions - 1);

    cpt = ctime(&session_info[sort_ixs[n]].gain_end_date);
    printf("%s\n",format_date(cpt));

    printf(fmt2,
      session_info[sort_ixs[n]].gain_amount,
      session_info[sort_ixs[n]].num_gain_sessions);

    if (!bVerbose)
      break;

    if (n < num_gains - 1)
      putchar(0x0a);
  }

  free(session_info);
  free(sort_ixs);

  if (bDebug) {
    printf("num_sessions = %4d\n",num_sessions);
    printf("num_gains   = %4d\n",num_gains);
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

  if (session_info[ix1].gain_amount !=
      session_info[ix2].gain_amount) {
    return session_info[ix2].gain_amount -
      session_info[ix1].gain_amount;
  }
  else  {
    return session_info[ix2].gain_start_date -
      session_info[ix1].gain_start_date;
  }
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
