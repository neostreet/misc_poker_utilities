#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];
static char save_line[MAX_LINE_LEN];

static char usage[] = "usage: blue_plunge filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int work;
  int last;
  int plunge_start;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(line,"%d",&work);

    if (!work)
      break;
  }

  if (feof(fptr)) {
    printf("no 0 line found\n");
    fclose(fptr);
    return 3;
  }

  last = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(line,"%d",&work);

    if (work < last) {
      printf("%s (%d)\n",save_line,line_no - plunge_start);

      if (work) {
        for ( ; ; ) {
          GetLine(fptr,line,&line_len,MAX_LINE_LEN);

          if (feof(fptr))
            break;

          line_no++;
          sscanf(line,"%d",&work);

          if (!work)
            break;
        }

        if (feof(fptr)) {
          fclose(fptr);
          return 0;
        }
      }

      last = 0;
      continue;
    }
    else {
      if (!last)
        plunge_start = line_no;
    }

    last = work;
    strcpy(save_line,line);
  }

  printf("%s (%d)\n",save_line,line_no - plunge_start + 1);

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
