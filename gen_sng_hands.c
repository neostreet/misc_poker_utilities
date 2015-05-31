#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: skeleton filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static void print_modified_line(char *line,int line_len);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;

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
    print_modified_line(line,line_len);
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

static void print_modified_line(char *line,int line_len)
{
  int n;

  printf("c:\\aidan\\pokerstars\\logs\\");

  for (n = 0; n < 4; n++)
    putchar(line[n]);

  putchar('\\');

  for (n = 0; n < 2; n++)
    putchar(line[5+n]);

  for (n = 0; n < 2; n++)
    putchar(line[8+n]);

  putchar('\\');

  putchar(line[10]);

  printf("\\sng_hands\n");
}
