#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define TAB 0x9

static char usage[] =
"usage: extrapolated_profit (-terse) (-verbose) (-offsetoffset)\n"
"  (-extrap_first) (-double) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static time_t cvt_date(char *date_str);

#define YEAR_IX  0
#define MONTH_IX 1
#define DAY_IX   2

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

#define SECS_PER_MIN  (double)60
#define MINS_PER_HOUR (double)60
#define HOURS_PER_DAY (double)24
#define SECS_PER_DAY (SECS_PER_MIN * MINS_PER_HOUR * HOURS_PER_DAY)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_and_delta(
  char *line,
  int line_len,
  time_t *date,
  int *delta
);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  int offset;
  bool bExtrapFirst;
  bool bDouble;
  FILE *fptr;
  int line_len;
  int line_no;
  int dbg_line_no;
  int dbg;
  int retval;
  char *cpt;
  time_t date;
  int delta;
  int year;
  static char date_buf[11];
  time_t start_date;
  time_t end_date;
  time_t datediff;
  int running_delta;
  double dwork;
  int days_in_year;
  int days_in_period;

  if ((argc < 2) || (argc > 7)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  offset = 0;
  bExtrapFirst = false;
  bDouble = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strncmp(argv[curr_arg],"-offset",7)) {
      sscanf(&argv[curr_arg][7],"%d",&offset);
    }
    else if (!strcmp(argv[curr_arg],"-extrap_first"))
      bExtrapFirst = true;
    else if (!strcmp(argv[curr_arg],"-double"))
      bDouble = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can't specify both -terse and -verbose\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;
  dbg_line_no = -1;
  running_delta = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    retval = get_date_and_delta(line,line_len,&date,&delta);

    if (retval) {
      printf("get_date_and_delta() failed on line %d: %d\n",
        line_no,retval);
      return 5;
    }

    if (line_no == 1) {
      sscanf(line,"%d",&year);
      sprintf(date_buf,"%d-12-31",year-1);
      start_date = cvt_date(date_buf);
      sprintf(date_buf,"%d-12-31",year);
      end_date = cvt_date(date_buf);
      datediff = end_date - start_date;
      dwork = (double)datediff / (double)SECS_PER_DAY;
      days_in_year = dwork;
    }

    running_delta += delta;

    datediff = date - start_date;
    dwork = (double)datediff / (double)SECS_PER_DAY;
    days_in_period = dwork;
    days_in_period += offset;

    dwork = ((double)days_in_year / (double)days_in_period) * (double)running_delta;

    if (line_no == dbg_line_no)
      dbg = 1;

    cpt = ctime(&date);

    if (!bVerbose) {
      if (bTerse) {
        if (!bDouble)
          printf("%d\n",(int)dwork);
        else
          printf("%lf\n",dwork);
      }
      else if (!bExtrapFirst) {
        if (!bDouble) {
          printf("%s %10d %10d %10d\n",
            format_date(cpt),delta,running_delta,(int)dwork);
        }
        else {
          printf("%s %10d %10d %lf\n",
            format_date(cpt),delta,running_delta,dwork);
        }
      }
      else {
        if (!bDouble) {
          printf("%10d %s %10d %10d\n",
            (int)dwork,format_date(cpt),delta,running_delta);
        }
        else {
          printf("%lf %s %d %d\n",
            dwork,format_date(cpt),delta,running_delta);
        }
      }
    }
    else if (!bExtrapFirst) {
      if (!bDouble) {
        printf("%s %10d %10d %10d (%5d %5d %10d)\n",
          format_date(cpt),delta,running_delta,(int)dwork,
          days_in_year,days_in_period,running_delta);
      }
      else {
        printf("%s %10d %10d %lf (%d %d %d)\n",
          format_date(cpt),delta,running_delta,dwork,
          days_in_year,days_in_period,running_delta);
      }
    }
    else {
      if (!bDouble) {
        printf("%10d %s %10d %10d (%5d %5d %10d)\n",
          (int)dwork,format_date(cpt),delta,running_delta,
          days_in_year,days_in_period,running_delta);
      }
      else {
        printf("%lf %s %d %d (%d %d %d)\n",
          dwork,format_date(cpt),delta,running_delta,
          days_in_year,days_in_period,running_delta);
      }
    }
  }

  fclose(fptr);

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

static int get_date_and_delta(
  char *line,
  int line_len,
  time_t *date,
  int *delta
)
{
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == TAB)
      break;
  }

  if (n == line_len)
    return 1;

  line[n++] = 0;

  *date = cvt_date(line);
  sscanf(&line[n],"%d",delta);

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
