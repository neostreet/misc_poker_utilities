#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: blue_distance2_simple filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  char str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;
  int blue_distance;
  int prev_blue_distance;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  balance = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%s\t%d",str,&delta);

    if (!line_no) {
      max_balance = delta;
      balance = max_balance;
      prev_blue_distance = 0;
    }
    else
      balance += delta;

    if (balance > max_balance)
      max_balance = balance;

    if (max_balance > 0)
      blue_distance = max_balance - balance;
    else
      blue_distance = max_balance * -1;

    printf("%d\t%s\t%d\n",blue_distance,line,prev_blue_distance);
    line_no++;
    prev_blue_distance = blue_distance;
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
