#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#define YEAR_IX  0
#define MONTH_IX 1
#define DAY_IX   2

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

struct session_info_struct {
  time_t poker_session_date;
  int delta;
};

struct rebound_struct {
  time_t poker_session_date;
  int rebound;
};

#define TAB 0x9

static char usage[] =
"usage: rebounds (-debug) (-no_sort) (-date_last) (-full) (-reverse)\n"
"  (-no_date) (-max) (-verbose) filename\n";
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

static char *months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
#define NUM_MONTHS (sizeof months / sizeof (char *))

static struct session_info_struct *session_info;
static struct rebound_struct *rebound;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_session_info(
  char *line,
  int line_len,
  struct session_info_struct *session_info,
  bool bNoDate);
static time_t cvt_date(char *date_str);
int elem_compare(const void *elem1,const void *elem2);
static char *format_date(char *cpt);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bNoSort;
  bool bDateLast;
  bool bFull;
  bool bReverse;
  bool bNoDate;
  bool bMax;
  bool bVerbose;
  int session_ix;
  FILE *fptr;
  int line_len;
  int set_size;
  int chara;
  int *sort_ixs;
  int retval;
  char *cpt;
  int num_rebounds;
  int is_rebound;
  int rebound_ix;
  int curr_rebound;

  if ((argc < 2) || (argc > 10)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bNoSort = false;
  bDateLast = false;
  bFull = false;
  bReverse = false;
  bNoDate = false;
  bMax = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = true;
    else if (!strcmp(argv[curr_arg],"-date_last"))
      bDateLast = true;
    else if (!strcmp(argv[curr_arg],"-full"))
      bFull = true;
    else if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = true;
    else if (!strcmp(argv[curr_arg],"-no_date"))
      bNoDate = true;
    else if (!strcmp(argv[curr_arg],"-max"))
      bMax = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bDateLast && bNoDate) {
    printf("can't specify both -date_last and -no_date\n");
    return 3;
  }

  if (bMax)
    bNoSort = false;

  if (bVerbose)
    getcwd(save_dir,_MAX_PATH);

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
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
    return 5;
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

    retval = get_session_info(line,line_len,&session_info[session_ix],bNoDate);

    if (retval) {
      printf("get_session_info() failed on line %d: %d\n",
        session_ix+1,retval);
      return 6;
    }

    session_ix++;
  }

  fclose(fptr);

  num_rebounds = 0;

  for (n = 1; n < set_size; n++) {
    if (!bReverse) {
      if ((session_info[n-1].delta < 0) && (session_info[n].delta > 0)) {
        if (!bFull)
          num_rebounds++;
        else if (session_info[n-1].delta * -1 >= session_info[n].delta)
          num_rebounds++;
      }
    }
    else {
      if ((session_info[n-1].delta > 0) && (session_info[n].delta < 0)) {
        if (!bFull)
          num_rebounds++;
        else if (session_info[n].delta * -1 >= session_info[n-1].delta)
          num_rebounds++;
      }
    }
  }

  if (bDebug)
    printf("num_rebounds = %d\n",num_rebounds);

  if ((rebound = (struct rebound_struct *)malloc(
    num_rebounds * sizeof (struct rebound_struct))) == NULL) {
    printf(malloc_failed2,num_rebounds);
    fclose(fptr);
    return 7;
  }

  if ((sort_ixs = (int *)malloc(
    num_rebounds * sizeof (int))) == NULL) {
    printf(malloc_failed3,num_rebounds);
    fclose(fptr);
    return 8;
  }

  rebound_ix = 0;

  for (n = 1; n < set_size; n++) {
    if (!bReverse) {
      if ((session_info[n-1].delta < 0) && (session_info[n].delta > 0)) {
        is_rebound = 0;

        if (!bFull)
          is_rebound = 1;
        else if (session_info[n-1].delta * -1 >= session_info[n].delta)
          is_rebound = 1;

        if (is_rebound) {
          curr_rebound = session_info[n-1].delta * -1;

          if (curr_rebound > session_info[n].delta)
            curr_rebound = session_info[n].delta;

          if (!bNoDate)
            rebound[rebound_ix].poker_session_date = session_info[n].poker_session_date;

          rebound[rebound_ix++].rebound = curr_rebound;
        }
      }
    }
    else {
      if ((session_info[n-1].delta > 0) && (session_info[n].delta < 0)) {
        is_rebound = 0;

        if (!bFull)
          is_rebound = 1;
        else if (session_info[n].delta * -1 >= session_info[n-1].delta)
          is_rebound = 1;

        if (is_rebound) {
          curr_rebound = session_info[n].delta * -1;

          if (curr_rebound > session_info[n-1].delta)
            curr_rebound = session_info[n-1].delta;

          if (!bNoDate)
            rebound[rebound_ix].poker_session_date = session_info[n].poker_session_date;

          rebound[rebound_ix++].rebound = curr_rebound;
        }
      }
    }
  }

  for (n = 0; n < num_rebounds; n++)
    sort_ixs[n] = n;

  if (!bNoSort)
    qsort(sort_ixs,num_rebounds,sizeof (int),elem_compare);

  for (n = 0; n < num_rebounds; n++) {
    if (bNoDate) {
      if (!bVerbose) {
        printf("%d\n",
          rebound[sort_ixs[n]].rebound);
      }
      else {
        printf("%d %s\n",
          rebound[sort_ixs[n]].rebound,save_dir);
      }
    }
    else {
      cpt = ctime(&rebound[sort_ixs[n]].poker_session_date);

      if (!bDateLast) {
        printf("%s %10d\n",
          format_date(cpt),rebound[sort_ixs[n]].rebound);
      }
      else {
        printf("%10d %s\n",
          rebound[sort_ixs[n]].rebound,format_date(cpt));
      }
    }

    if (bMax)
      break;
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
  struct session_info_struct *session_info,
  bool bNoDate)
{
  int m;
  int n;

  if (bNoDate)
    n = 0;
  else {
    for (n = 0; n < line_len; n++) {
      if (line[n] == TAB)
        break;
    }

    if (n == line_len)
      return 1;

    line[n++] = 0;

    session_info->poker_session_date = cvt_date(line);
  }

  sscanf(&line[n],"%d",&session_info->delta);

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
