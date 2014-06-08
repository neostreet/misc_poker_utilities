#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

struct delta_info_struct {
  time_t start_date;
  int start_hand;
  time_t end_date;
  int end_hand;
  int delta;
  int sum;
  int num_deltas;
};

#define TAB 0x9

static char usage[] =
"usage: delta_streaks (-ascending) (-sort_by_sum_delta) (-total)\n"
"  (-only_winning) (-only_losing) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char malloc_failed1[] = "malloc of %d delta info structures failed\n";
static char malloc_failed2[] = "malloc of %d ints failed\n";

static char *months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
#define NUM_MONTHS (sizeof months / sizeof (char *))

static struct delta_info_struct *delta_info;
static struct delta_info_struct *streaks;
static bool bAscending;
static bool bSortBySumDelta;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_delta_info(
  char *line,
  int line_len,
  struct delta_info_struct *delta_info);
static time_t cvt_date(char *line,int line_len);
static int get_hand(char *line,int line_len);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int m;
  int n;
  bool bTotal;
  bool bOnlyWinning;
  bool bOnlyLosing;
  int curr_arg;
  FILE *fptr;
  int line_len;
  int num_deltas;
  int delta_ix;
  int delta;
  int prev_delta;
  int chara;
  int num_streaks;
  int streak_ix;
  int retval;
  char *cpt;
  int total_deltas;
  int total_delta_deltas;

  if ((argc < 2) || (argc > 7)) {
    printf(usage);
    return 1;
  }

  bTotal = false;
  bOnlyWinning = false;
  bOnlyLosing = false;
  bAscending = false;
  bSortBySumDelta = false;

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

  num_deltas = 0;
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

    line[10] = 0;
    sscanf(&line[0],"%d",&delta);

    if ((num_deltas == 0) || ((delta > 0) && (prev_delta <= 0)) ||
        ((delta <= 0) && (prev_delta > 0)))
      num_streaks++;

    num_deltas++;
    prev_delta = delta;
  }

  if ((delta_info = (struct delta_info_struct *)malloc(
    num_deltas * sizeof (struct delta_info_struct))) == NULL) {
    printf(malloc_failed1,num_deltas);
    fclose(fptr);
    return 5;
  }

  if ((streaks = (struct delta_info_struct *)malloc(
    num_streaks * sizeof (struct delta_info_struct))) == NULL) {
    printf(malloc_failed1,num_streaks);
    fclose(fptr);
    free(delta_info);
    return 6;
  }

  fseek(fptr,0L,SEEK_SET);

  delta_ix = 0;

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

    retval = get_delta_info(line,line_len,&delta_info[delta_ix]);

    if (retval) {
      printf("get_delta_info() failed on line %d: %d\n",
        delta_ix+1,retval);
      fclose(fptr);
      free(delta_info);
      free(streaks);
      return 7;
    }

    delta_ix++;
  }

  fclose(fptr);

  streak_ix = 0;

  for (n = 0; n < num_deltas; n++) {
    if ((n == 0) || ((delta_info[n].delta > 0) && (delta_info[n-1].delta <= 0)) ||
        ((delta_info[n].delta <= 0) && (delta_info[n-1].delta > 0))) {
      streaks[streak_ix].start_date = delta_info[n].start_date;
      streaks[streak_ix].start_hand = delta_info[n].start_hand;

      streaks[streak_ix].sum = delta_info[n].delta;

      for (m = n + 1; (m < num_deltas) && (((delta_info[n].delta > 0) && (delta_info[m].delta > 0)) ||
        ((delta_info[n].delta <= 0) && (delta_info[m].delta <= 0))); m++)
        streaks[streak_ix].sum += delta_info[m].delta;

      streaks[streak_ix].end_date = delta_info[m - 1].start_date;
      streaks[streak_ix].end_hand = delta_info[m - 1].start_hand;
      streaks[streak_ix].num_deltas = m - n;

      streak_ix++;
    }
  }

  if (bTotal) {
    total_deltas = 0;
    total_delta_deltas = 0;
  }

  for (n = 0; n < num_streaks; n++) {
    if (bOnlyWinning && streaks[n].sum <= 0)
      continue;

    if (bOnlyLosing && streaks[n].sum > 0)
      continue;

    printf("%10d %3d ",streaks[n].sum,streaks[n].num_deltas);

    cpt = ctime(&streaks[n].start_date);
    printf("%s %d ",format_date(cpt),streaks[n].start_hand);

    cpt = ctime(&streaks[n].end_date);
    printf("%s %d\n",format_date(cpt),streaks[n].end_hand);

    if (bTotal) {
      total_deltas += streaks[n].num_deltas;
      total_delta_deltas += streaks[n].sum;
    }
  }

  free(delta_info);
  free(streaks);

  if (bTotal) {
    printf("\n%3d                       %10d\n",
      total_deltas,total_delta_deltas);
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

static int get_delta_info(
  char *line,
  int line_len,
  struct delta_info_struct *delta_info)
{
  int n;
  int work;

  delta_info->start_date = cvt_date(line,line_len);
  delta_info->start_hand = get_hand(line,line_len);

  line[10] = 0;
  sscanf(&line[0],"%d",&work);

  delta_info->delta = work;

  return 0;
}

static time_t cvt_date(char *line,int line_len)
{
  int n;
  struct tm tim;
  time_t ret_tm;
  char buf[10];

  if (line_len < 52)
    return -1L;

  for (n = 0; n < 4; n++)
    buf[n] = line[42+n];

  buf[n] = 0;

  tim.tm_year = atoi(buf) - 1900;

  for (n = 0; n < 2; n++)
    buf[n] = line[47+n];

  buf[n] = 0;

  tim.tm_mon = atoi(buf) - 1;

  for (n = 0; n < 2; n++)
    buf[n] = line[49+n];

  buf[n] = 0;

  tim.tm_mday = atoi(buf);

  tim.tm_hour = 0;
  tim.tm_min = 0;
  tim.tm_sec = 0;

  tim.tm_isdst = 0;

  ret_tm = mktime(&tim);

  return ret_tm;
}

static int get_hand(char *line,int line_len)
{
  int n;

  for (n = line_len - 1; (n >= 0); n--) {
    if (line[n] == ' ')
      break;
  }

  return atoi(&line[n+1]);
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
