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

static char usage[] =
"usage: missing_dates (-length) (-ctime) (-not_missing) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

struct missing_dates_info {
  time_t start_date;
  time_t end_date;
  int datediff;
};

static struct missing_dates_info *missing_dates;

static char malloc_failed1[] = "malloc of %d missing_dates_info structures failed\n";
static char malloc_failed2[] = "malloc of %d ints failed\n";

#define SECS_PER_MIN  60
#define MINS_PER_HOUR 60
#define HOURS_PER_DAY 24
#define SECS_PER_DAY (SECS_PER_MIN * MINS_PER_HOUR * HOURS_PER_DAY)

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
static int get_session_date(char *line,time_t *session_date);
static time_t cvt_date(char *date_str);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bLength;
  bool bCtime;
  bool bNotMissing;
  int m;
  int n;
  FILE *fptr;
  int line_len;
  int num_sessions;
  int session_ix;
  int retval;
  time_t missing_date;
  char *cpt;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bLength = false;
  bCtime = false;
  bNotMissing = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-length"))
      bLength = true;
    else if (!strcmp(argv[curr_arg],"-ctime"))
      bCtime = true;
    else if (!strcmp(argv[curr_arg],"-not_missing"))
      bNotMissing = true;
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

  if ((missing_dates = (struct missing_dates_info *)malloc(
    num_sessions * sizeof (struct missing_dates_info))) == NULL) {
    printf(malloc_failed1,num_sessions);
    fclose(fptr);
    return 4;
  }

  session_ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    retval = get_session_date(line,&missing_dates[session_ix].start_date);

    if (retval) {
      printf("get_session_date() failed on line %d: %d\n",session_ix+1,retval);
      return 5;
    }

    session_ix++;
  }

  fclose(fptr);

  for (n = 0; n < num_sessions; n++) {
    if (!n)
      missing_dates[n].datediff = 0;
    else
      missing_dates[n].datediff = (missing_dates[n].start_date - missing_dates[n - 1].start_date) /
        (SECS_PER_DAY);
  }

  for (n = 0; n < num_sessions; n++) {
    if (!n || (missing_dates[n].datediff == 1)) {
      if (bNotMissing) {
        if (!bLength) {
          missing_date = missing_dates[n].start_date;
          cpt = ctime(&missing_date);

          if (bCtime)
            printf("1 %s",cpt);
          else
            printf("1 %s\n",format_date(cpt));
        }
      }

      continue;
    }

    if (missing_dates[n].datediff > 1) {
      missing_date = missing_dates[n-1].start_date;

      if (!bLength) {
        for (m = 0; m < missing_dates[n].datediff - 1; m++) {
          missing_date += SECS_PER_DAY;
          cpt = ctime(&missing_date);

          if (bCtime) {
            if (!bNotMissing)
              printf("%s",cpt);
            else
              printf("0 %s",cpt);
          }
          else {
            if (!bNotMissing)
              printf("%s\n",format_date(cpt));
            else
              printf("0 %s\n",format_date(cpt));
          }
        }

        if (bNotMissing) {
          missing_date += SECS_PER_DAY;
          cpt = ctime(&missing_date);
          printf("1 %s\n",format_date(cpt));
        }
      }
      else {
        cpt = ctime(&missing_date);

        if (bCtime)
          printf("%3d %s",missing_dates[n].datediff - 1,cpt);
        else
          printf("%3d %s\n",missing_dates[n].datediff - 1,format_date(cpt));
      }
    }
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

static int get_session_date(char *line,time_t *session_date)
{
  *session_date = cvt_date(line);

  if (*session_date == -1L)
    return 1;

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
