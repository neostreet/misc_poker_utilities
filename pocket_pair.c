#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: pocket_pair (-debug) denomination filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  int bDebug;
  FILE *fptr;
  int line_len;
  int line_no;
  int rocket_count;
  int last_rocket_line;
  double rocket_pct;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  line_no = 0;
  rocket_count = 0;

  if (bDebug)
    last_rocket_line = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if ((line_len >= 5) && (line[0] == argv[curr_arg][0]) && (line[3] == argv[curr_arg][0])) {
      rocket_count++;

      if (bDebug) {
        if (last_rocket_line == -1)
          printf("%10d       %s\n",line_no,line);
        else
          printf("%10d (%3d) %s\n",line_no,line_no - last_rocket_line,line);

        last_rocket_line = line_no;
      }
    }
  }

  if (bDebug && (rocket_count > 0))
    putchar(0x0a);

  rocket_pct = (double)rocket_count * (double)100 / (double)line_no;

  printf("%d of %d hands (%6.3lf%%)\n",rocket_count,line_no,rocket_pct);

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
