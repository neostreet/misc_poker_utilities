#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 8192
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: all_under_to_all_above (-reverse) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bReverse;
  FILE *fptr;
  int line_len;
  int line_no;
  double pct;
  double save_pct;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bReverse = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = true;
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

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%lf",&pct);

    if (line_no > 1) {
      if (!bReverse) {
        if ((save_pct == (double)1) && (pct == (double)0))
          printf("%s\n",line);
      }
      else {
        if ((save_pct == (double)0) && (pct == (double)1))
          printf("%s\n",line);
      }
    }

    save_pct = pct;
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
