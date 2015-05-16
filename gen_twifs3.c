#include <stdio.h>
#include <stdlib.h>

static char usage[] = "usage: gen_twifs3 filename\n";
static char couldnt_open[] = "couldn't open %s\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define BUF_LEN 1024
static char buf[BUF_LEN];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int firsts;
  int seconds;
  int total;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %d %d",&firsts,&seconds,&total);
    sprintf(buf,"echo %d %d %d",firsts,seconds,total);
    system(buf);
    sprintf(buf,"tournament_wif /cygdrive/c/users/alloyd/misc/git/misc_poker_utilities/50000a %d %d %d",firsts,seconds,total);
    system(buf);
    sprintf(buf,"tournament_wif /cygdrive/c/users/alloyd/misc/git/misc_poker_utilities/50000b %d %d %d",firsts,seconds,total);
    system(buf);
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
