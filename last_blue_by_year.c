#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

struct blue_info {
  char date[11];
  int balance;
};

#define MAX_YEARS 50
static struct blue_info my_blue_info[MAX_YEARS];

static char usage[] = "usage: last_blue_by_year filename\n";
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
    my_blue_info[n].balance = 0;
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
        line[10] = 0;
        strcpy(my_blue_info[year - first_year].date,line);
        my_blue_info[year - first_year].balance = work;
      }

      max = work;
    }
  }

  fclose(fptr);

  for (n = 0; n < MAX_YEARS; n++) {
    if (my_blue_info[n].balance)
      printf("%s %d\n",my_blue_info[n].date,my_blue_info[n].balance);
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

static int get_year_and_balance(char *line,int line_len,int *year,int *balance)
{
  if (sscanf(line,"%d",year) != 1)
    return 1;

  if (sscanf(&line[11],"%d",balance) != 1)
    return 2;

  return 0;
}
