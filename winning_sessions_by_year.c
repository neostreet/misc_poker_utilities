#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_YEARS 50
static int winning_sessions[MAX_YEARS];
static int year_count[MAX_YEARS];

static char usage[] =
"usage: winning_sessions_by_year (-offsetoffset) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_year_and_balance(char *line,int line_len,int *year,int *balance);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int offset;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  int first_year;
  int year;
  int work;
  double winning_pct;
  int tot_winning_sessions;
  int tot_year_count;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  offset = 0;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strncmp(argv[curr_arg],"-offset",7))
      sscanf(&argv[curr_arg][7],"%d",&offset);
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

  for (n = 0; n < MAX_YEARS; n++) {
    winning_sessions[n] = 0;
    year_count[n] = 0;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    retval = get_year_and_balance(line,line_len,&year,&work);

    if (retval) {
      printf("get_year_and_balance() failed on line %d: %d\n",line_no,retval);
      return 4;
    }

    if (line_no == 1)
      first_year = year;

    if (year - first_year >= MAX_YEARS) {
      printf("line %d: year %d outside of range\n",line_no, year);
      return 5;
    }

    year_count[year - first_year]++;

    if (work > 0)
      winning_sessions[year - first_year]++;
  }

  fclose(fptr);

  tot_winning_sessions = 0;
  tot_year_count = 0;

  for (n = 0; n < MAX_YEARS; n++) {
    if (winning_sessions[n]) {
      winning_pct = (double)winning_sessions[n] / (double)year_count[n];

      printf("%d: %d of %d (%lf)\n",first_year + n,winning_sessions[n],year_count[n],winning_pct);

      tot_winning_sessions += winning_sessions[n];
    }

    tot_year_count += year_count[n];
  }

  putchar(0x0a);

  winning_pct = (double)tot_winning_sessions / (double)tot_year_count;

  printf("%d of %d (%lf)\n",tot_winning_sessions,tot_year_count,winning_pct);

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

static int get_year_and_balance(char *line,int line_len,int *year,int *balance)
{
  if (sscanf(line,"%d",year) != 1)
    return 1;

  if (sscanf(&line[11],"%d",balance) != 1)
    return 2;

  return 0;
}
