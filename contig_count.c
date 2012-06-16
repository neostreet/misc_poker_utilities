#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: contig_count (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  int bVerbose;
  FILE *fptr;
  int line_len;
  int line_no;
  int val;
  int last_val;
  int contig_count;
  int contig_start;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = TRUE;
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

    sscanf(line,"%d",&val);

    if (line_no == 1) {
      last_val = val;
      contig_count = 1;

      if (bVerbose)
        contig_start = 1;
    }
    else if (val == last_val)
      contig_count++;
    else {
      if (!bVerbose)
        printf("%d %4d\n",last_val,contig_count);
      else {
        printf("%d %4d (%4d %4d)\n",last_val,contig_count,
          contig_start,contig_start + contig_count - 1);
        contig_start += contig_count;
      }

      last_val = val;
      contig_count = 1;
    }
  }

  fclose(fptr);

  if (!bVerbose)
    printf("%d %4d\n",last_val,contig_count);
  else {
    printf("%d %4d (%4d %4d)\n",last_val,contig_count,
      contig_start,contig_start + contig_count - 1);
  }

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
