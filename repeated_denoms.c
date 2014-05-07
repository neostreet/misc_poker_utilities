#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];
static char prev_line[MAX_LINE_LEN];

static char usage[] = "usage: repeated_denoms (-dont_print_prev) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char fmt_str[] = "%s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bDontPrintPrev;
  FILE *fptr;
  int line_len;
  int line_no;
  char prev_denoms[2];
  char curr_denoms[2];
  bool bPrintedPrev;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDontPrintPrev = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-dont_print_prev"))
      bDontPrintPrev = true;
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
  bPrintedPrev = false;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    curr_denoms[0] = line[11];
    curr_denoms[1] = line[14];

    if (line_no > 1) {
      for (m = 0; m < 2; m++) {
        for (n = 0; n < 2; n++) {
          if (curr_denoms[m] == prev_denoms[n])
            break;
        }

        if (n < 2)
          break;
      }

      if (m < 2) {
        if (!bDontPrintPrev && !bPrintedPrev) {
          printf(fmt_str,prev_line);
          bPrintedPrev = true;
        }

        printf(fmt_str,line);
      }
      else
        bPrintedPrev = false;
    }

    strcpy(prev_line,line);

    for (n = 0; n < 2; n++)
      prev_denoms[n] = curr_denoms[n];
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
