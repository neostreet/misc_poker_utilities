#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_YEARS 50
static int blue_count[MAX_YEARS];

static char usage[] = "usage: first_blue_by_year filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_year_and_balance(char *line,int line_len,int *year,int *balance);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  int first_year;
  int year;
  int work;
  int max;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  for (n = 0; n < MAX_YEARS; n++) {
    blue_count[n] = 0;
  }

  line_no = 0;
  max = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    retval = get_year_and_balance(line,line_len,&year,&work);

    if (retval) {
      printf("get_year_and_balance() failed on line %d: %d\n",line_no,retval);
      return 3;
    }

    if (line_no == 1)
      first_year = year;

    if (year - first_year >= MAX_YEARS) {
      printf("line %d: year %d outside of range\n",line_no, year);
      return 4;
    }

    if (work > max) {
      if (line_no > 1) {
        blue_count[year - first_year]++;

        if (blue_count[year - first_year] == 1)
          printf("%s\n",line);
      }

      max = work;
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

static int get_year_and_balance(char *line,int line_len,int *year,int *balance)
{
  if (sscanf(line,"%d",year) != 1)
    return 1;

  if (sscanf(&line[11],"%d",balance) != 1)
    return 2;

  return 0;
}
