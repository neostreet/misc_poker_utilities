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

struct streak_info_struct {
  int sum;
  int streak_len;
  time_t start_date;
  int start_hand;
  time_t end_date;
  int end_hand;
};

static char usage[] = "usage: loneliest_pot filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char malloc_failed1[] = "malloc of %d streak info structures failed\n";

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

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_streak_info(
  char *line,
  int line_len,
  struct streak_info_struct *streak_info);
static time_t cvt_date(char *date_str);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  struct streak_info_struct *streak_info;
  int num_streaks;
  int streak_ix;
  int retval;
  char *cpt;
  int max_losing_streak;
  int loneliest_ix;
  int curr_losing_streak;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
  }

  num_streaks = line_no;

  fseek(fptr,0L,SEEK_SET);

  streak_ix = 0;

  if ((streak_info = (struct streak_info_struct *)malloc(
    num_streaks * sizeof (struct streak_info_struct))) == NULL) {
    printf(malloc_failed1,num_streaks);
    fclose(fptr);
    return 3;
  }

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    retval = get_streak_info(line,line_len,&streak_info[streak_ix]);

    if (retval) {
      printf("get_streak_info() failed on line %d: %d\n",
        streak_ix+1,retval);
      fclose(fptr);
      free(streak_info);
      return 4;
    }

    streak_ix++;
  }

  fclose(fptr);

  max_losing_streak = 1;
  loneliest_ix = -1;

  for (n = 1; n < num_streaks - 1; n++) {
    if ((streak_info[n].sum > 0) && (streak_info[n].streak_len == 1)) {
      if (streak_info[n-1].streak_len < streak_info[n+1].streak_len)
        curr_losing_streak = streak_info[n-1].streak_len;
      else
        curr_losing_streak = streak_info[n+1].streak_len;

      if ((max_losing_streak == -1) || (curr_losing_streak > max_losing_streak)) {
        max_losing_streak = curr_losing_streak;
        loneliest_ix = n;
      }
    }
  }

  for (n = loneliest_ix - 1; n <= loneliest_ix + 1; n++) {
    cpt = ctime(&streak_info[n].start_date);
    printf("%10d %3d %s %3d ",
      streak_info[n].sum,streak_info[n].streak_len,
      format_date(cpt),streak_info[n].start_hand);
    cpt = ctime(&streak_info[n].end_date);
    printf("%s %3d\n",
      format_date(cpt),streak_info[n].end_hand);
  }

  free(streak_info);

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

static int get_streak_info(
  char *line,
  int line_len,
  struct streak_info_struct *streak_info)
{
  line[10] = 0;
  streak_info->sum = atoi(&line[0]);

  line[14] = 0;
  streak_info->streak_len = atoi(&line[11]);

  line[25] = 0;
  streak_info->start_date = cvt_date(&line[15]);

  line[29] = 0;
  streak_info->start_hand = atoi(&line[26]);

  line[40] = 0;
  streak_info->end_date = cvt_date(&line[30]);

  streak_info->end_hand = atoi(&line[41]);

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
