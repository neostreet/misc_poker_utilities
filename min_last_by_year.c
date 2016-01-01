#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

struct min_last {
  int have_min;
  int min;
  int last;
};

#define MAX_YEARS 50
static struct min_last min_last_by_year[MAX_YEARS];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: min_last_by_year (-verbose) filename\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  FILE *fptr;
  int line_len;
  int line_no;
  int work;
  int first_year;
  int curr_year;
  int year_ix;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf("couldn't open %s\n",argv[curr_arg]);
    return 3;
  }

  for (n = 0; n < MAX_YEARS; n++) {
    min_last_by_year[n].have_min = 0;
    min_last_by_year[n].last = 0;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(&line[0],"%d",&curr_year);
    sscanf(&line[11],"%d",&work);

    if (line_no == 1)
      first_year = curr_year;

    year_ix = curr_year - first_year;

    if (year_ix >= MAX_YEARS) {
      printf("MAX_YEARS of %d exceeded on line %d\n",MAX_YEARS,line_no);
      return 4;
    }

    min_last_by_year[year_ix].last += work;

    if (!min_last_by_year[year_ix].have_min ||
       (min_last_by_year[year_ix].last < min_last_by_year[year_ix].min)) {
      min_last_by_year[year_ix].min = min_last_by_year[year_ix].last;
      min_last_by_year[year_ix].have_min = 1;
    }
  }

  fclose(fptr);

  for (n = 0; n < MAX_YEARS; n++) {
    if (min_last_by_year[n].have_min) {
      if (!bVerbose) {
        printf("%d: %d\n",first_year + n,
          min_last_by_year[n].last - min_last_by_year[n].min);
      }
      else {
        printf("%d: %d (%d %d)\n",first_year + n,
          min_last_by_year[n].last - min_last_by_year[n].min,
          min_last_by_year[n].last,min_last_by_year[n].min);
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
