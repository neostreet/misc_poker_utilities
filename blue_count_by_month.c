#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_MONTHS 600
static int blue_count[MAX_MONTHS];
static int month_count[MAX_MONTHS];
static int years[MAX_MONTHS];
static int months[MAX_MONTHS];

static char usage[] =
"usage: blue_count_by_month (-offsetoffset) (-pct_first) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_year_month_mon_and_balance(char *line,int line_len,int *year,int *month,int *mon,int *balance);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int offset;
  bool bPctFirst;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  int year;
  int month;
  int mon;
  int first_mon;
  int work;
  int max;
  double blue_pct;
  int tot_blue_count;
  int tot_month_count;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  offset = 0;
  bPctFirst = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strncmp(argv[curr_arg],"-offset",7))
      sscanf(&argv[curr_arg][7],"%d",&offset);
    else if (!strcmp(argv[curr_arg],"-pct_first"))
      bPctFirst = true;
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

  for (n = 0; n < MAX_MONTHS; n++) {
    blue_count[n] = 0;
    month_count[n] = 0;
  }

  line_no = 0;
  max = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    retval = get_year_month_mon_and_balance(line,line_len,&year,&month,&mon,&work);

    if (retval) {
      printf("get_year_month_mon_and_balance() failed on line %d: %d\n",line_no,retval);
      return 4;
    }

    if (line_no == 1)
      first_mon = mon;

    if (mon - first_mon >= MAX_MONTHS) {
      printf("line %d: month %d outside of range\n",line_no,mon);
      return 5;
    }

    month_count[mon - first_mon]++;
    years[mon - first_mon] = year;
    months[mon - first_mon] = month;

    if (work > max) {
      if (line_no > 1)
        blue_count[mon - first_mon]++;

      max = work;
    }
  }

  fclose(fptr);

  tot_blue_count = 0;
  tot_month_count = 0;

  for (n = 0; n < MAX_MONTHS; n++) {
    if (blue_count[n]) {
      blue_pct = (double)blue_count[n] / (double)month_count[n];

      if (!bPctFirst)
        printf("%d-%d: %d of %d (%lf)\n",years[n],months[n],blue_count[n],month_count[n],blue_pct);
      else
        printf("%lf %d-%d: %d of %d\n",blue_pct,years[n],months[n],blue_count[n],month_count[n]);

      tot_blue_count += blue_count[n];
    }

    tot_month_count += month_count[n];
  }

  putchar(0x0a);

  blue_pct = (double)tot_blue_count / (double)tot_month_count;

  if (!bPctFirst)
    printf("%d of %d (%lf)\n",tot_blue_count,tot_month_count,blue_pct);
  else
    printf("%lf %d of %d\n",blue_pct,tot_blue_count,tot_month_count);

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

static int get_year_month_mon_and_balance(char *line,int line_len,int *year,int *month,int *mon,int *balance)
{
  if (sscanf(line,"%d",year) != 1)
    return 1;

  if (sscanf(&line[5],"%d",month) != 1)
    return 2;

  *mon = *year * 12 + *month - 1;

  if (sscanf(&line[11],"%d",balance) != 1)
    return 3;

  return 0;
}
