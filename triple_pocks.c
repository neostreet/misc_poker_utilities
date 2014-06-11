#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: triple_pocks (-exact) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static bool triple_pocks(char *line,int line_len,bool bExact);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bExact;
  FILE *fptr;
  int line_len;
  int line_no;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bExact = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-exact"))
      bExact = true;
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

    if (triple_pocks(line,line_len,bExact))
      printf("%s\n",line);
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

static bool triple_pocks(char *line,int line_len,bool bExact)
{
  int n;
  int hands_count;
  int pocks_count;

  hands_count = 0;
  pocks_count = 0;

  for (n = 0; n < line_len; n++) {
    if (line[n] == '[') {
      hands_count++;

      if (line[n+1] == line[n+4])
        pocks_count++;

      n += 6;
    }
  }

  if (bExact) {
    if ((hands_count == 3) && (pocks_count == 3))
      return true;
  }
  else {
    if (pocks_count >= 3)
      return true;
  }

  return false;
}
