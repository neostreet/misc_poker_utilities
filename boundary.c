#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: boundary (-rising) boundary filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bRising;
  FILE *fptr;
  int line_len;
  int line_no;
  int boundary;
  char date_str[20];
  int starting_balance;
  int ending_balance;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bRising = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-rising"))
      bRising = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&boundary);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%s\t%d\t%d",&date_str[0],&starting_balance,&ending_balance);

    if (((starting_balance < boundary) && (boundary < ending_balance)) ||
        (!bRising && (starting_balance > boundary) && (boundary > ending_balance))) {
      printf("%s\t%d\t%d\n",date_str,starting_balance,ending_balance);
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
