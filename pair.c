#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: pair (-debug) (-offsetoffset) denomination filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  int bDebug;
  int offset;
  FILE *fptr;
  int line_len;
  int line_no;
  int pair_count;
  double pair_pct;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = FALSE;
  offset = 0;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
    else if (!strncmp(argv[curr_arg],"-offset",7))
      sscanf(&argv[curr_arg][7],"%d",&offset);
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
  pair_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if ((line_len >= 5) &&
        (line[offset + 0] == argv[curr_arg][0]) &&
        (line[offset + 3] == argv[curr_arg][0])) {
      pair_count++;

      if (bDebug)
        printf("%s\n",line);
    }
  }

  if (bDebug && (pair_count > 0))
    putchar(0x0a);

  pair_pct = (double)pair_count * (double)100 / (double)line_no;

  printf("%d of %d hands (%6.3lf%%)\n",pair_count,line_no,pair_pct);

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
