#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: is_pos (-terse) (-verbose) (-date_first) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

#define DATE_LEN 11

int main(int argc,char **argv)
{
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  bool bDateFirst;
  FILE *fptr;
  int line_len;
  int line_no;
  int work;
  char date[DATE_LEN];

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  bDateFirst = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-date_first"))
      bDateFirst = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can only specify one of -terse and -verbose\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d %s",&work,date);

    if (work > 0) {
      if (bTerse)
        printf("1\n");
      else if (bVerbose)
          printf("1 %s\n",line);
      else {
        if (!bDateFirst)
          printf("1\t%s\n",date);
        else
          printf("%s\t1\n",date);
      }
    }
    else {
      if (bTerse)
        printf("0\n");
      else if (bVerbose)
          printf("0 %s\n",line);
      else {
        if (!bDateFirst)
          printf("0\t%s\n",date);
        else
          printf("%s\t0\n",date);
      }
    }

    line_no++;
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
