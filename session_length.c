#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: session_length (-verbose) (-seconds) (-avg) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char invalid_date_and_time[] = "%s %s: invalid date and time\n";

static char pokerstars[] = "PokerStars ";
#define POKERSTARS_LEN (sizeof (pokerstars) - 1)

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
  1, 31      /* day */
};

static struct digit_range time_checks[3] = {
  0, 23,  /* hour */
  0, 59,     /* minute */
  0, 59      /* second */
};

static time_t cvt_date_and_time(char *date_str,char *time_str);

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bSeconds;
  bool bAvg;
  FILE *fptr;
  int line_len;
  int line_no;
  int num_hands;
  int ix;
  time_t date1;
  time_t date2;
  int seconds;
  int diff;
  int hours;
  double avg;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bSeconds = false;
  bAvg = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-seconds"))
      bSeconds = true;
    else if (!strcmp(argv[curr_arg],"-avg"))
      bAvg = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 2;
  }

  line_no = 0;
  num_hands = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (Contains(true,
      line,line_len,
      pokerstars,POKERSTARS_LEN,
      &ix)) {

      num_hands++;

      line[line_len - 13] = 0;
      line[line_len - 4] = 0;

      date2 = cvt_date_and_time(&line[line_len - 23],&line[line_len - 12]);

      if (date2 == -1L) {
        printf(invalid_date_and_time,&line[line_len - 23],&line[line_len - 12]);
        return 3;
      }

      if (line_no == 1)
        date1 = date2;
    }
  }

  fclose(fptr);

  seconds = date2 - date1;

  if (bSeconds || (seconds < 60))
    printf("%d seconds",seconds);
  else {
    diff = seconds;

    if (diff < 60 * 60)
      printf("%d minutes, %d seconds",diff / 60,diff % 60);
    else {
      hours = diff / (60 * 60);
      diff -= (hours * 60 * 60);
      printf("%d hours, %d minutes, %d seconds",hours,diff / 60,diff %60);
    }
  }

  if (bAvg) {
    avg = (double)seconds / (double)(num_hands - 1);

    printf(" (%lf)",avg);
  }

  if (!bVerbose)
    putchar(0x0a);
  else
    printf(" %s\n",save_dir);

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

static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index)
{
  int m;
  int n;
  int tries;
  char chara;

  tries = line_len - string_len + 1;

  if (tries <= 0)
    return false;

  for (m = 0; m < tries; m++) {
    for (n = 0; n < string_len; n++) {
      chara = line[m + n];

      if (!bCaseSens) {
        if ((chara >= 'A') && (chara <= 'Z'))
          chara += 'a' - 'A';
      }

      if (chara != string[n])
        break;
    }

    if (n == string_len) {
      *index = m;
      return true;
    }
  }

  return false;
}

static time_t cvt_date_and_time(char *date_str,char *time_str)
{
  struct tm tim;
  char hold[11];
  int date_len;
  int time_len;
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
      if (date_str[bufix] == '/') {
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

  time_len = strlen(time_str);

  if (!time_len || (time_len > 8))
    return -1L;

  bufix = 0;

  for (n = 0; n < 3; n++) {
    holdix = 0;

    for ( ; bufix < time_len; ) {
      if (time_str[bufix] == ':') {
        bufix++;
        break;
      }

      if ((time_str[bufix] < '0') || (time_str[bufix] > '9'))
        return -1L;

      hold[holdix++] = time_str[bufix++];
    }

    if (!holdix || ((n != 2) && (bufix == time_len)))
      return -1L;

    hold[holdix] = 0;
    digits[n] = atoi(hold);

    if ((digits[n] > time_checks[n].upper) ||
      (digits[n] < time_checks[n].lower))
      return -1L;
  }

  tim.tm_hour = digits[0];
  tim.tm_min = digits[1];
  tim.tm_sec = digits[2];

  tim.tm_isdst = 0;

  ret_tm = mktime(&tim);

  return ret_tm;
}
