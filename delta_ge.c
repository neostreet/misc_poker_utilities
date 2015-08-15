#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: delta_ge (-verbose) ge_val filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int ge_val;
  FILE *fptr;
  int line_len;
  int line_no;
  int last_ge_line_no;
  int val;
  int intervening;
  int max_intervening;

  if ((argc < 3) || (argc > 4)) {
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

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&ge_val);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  line_no = 0;
  last_ge_line_no = 0;
  max_intervening = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&val);

    if (val >= ge_val) {
      if (!bVerbose)
        printf("%d\n",val);
      else if (!last_ge_line_no)
        printf("%s %d\n",line,line_no);
      else {
        intervening = line_no - (last_ge_line_no + 1);

        if (intervening > max_intervening)
          max_intervening = intervening;

        printf("%s %d (%d)\n",line,line_no,intervening);
      }

      last_ge_line_no = line_no;
    }
  }

  fclose(fptr);

  printf("max_intervening = %d\n",max_intervening);

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
