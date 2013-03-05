#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: hand_grep (-debug) rank1 rank2 filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  FILE *fptr;
  int line_len;
  int line_no;

  if ((argc < 4) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else
      break;
  }

  if (argc - curr_arg != 3) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg+2],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+2]);
    return 3;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (((line[0] == argv[curr_arg][0]) && (line[3] == argv[curr_arg+1][0])) ||
        ((line[0] == argv[curr_arg+1][0]) && (line[3] == argv[curr_arg][0]))) {
      if (!bDebug)
        printf("%s\n",line);
      else
        printf("%7d: %s\n",line_no,line);
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
