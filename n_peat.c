#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: n_peat offset len target_repeat_count filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int offset;
  int len;
  int target_repeat_count;
  int repeat_count;
  FILE *fptr;
  int line_len;
  int line_no;
  char last_hand[3];

  if (argc != 5) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&offset);
  sscanf(argv[2],"%d",&len);
  sscanf(argv[3],"%d",&target_repeat_count);

  if ((fptr = fopen(argv[4],"r")) == NULL) {
    printf(couldnt_open,argv[4]);
    return 2;
  }

  line_no = 0;
  repeat_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (line_len < offset + len) {
      printf("line %d is too short\n",line_no);
      return 3;
    }

    if (line_no == 1)
      repeat_count = 1;
    else {
      for (n = 0; n < len; n++) {
        if (line[offset+n] != last_hand[n])
          break;
      }

      if (n < len)
        repeat_count = 1;
      else {
        repeat_count++;

        if (repeat_count >= target_repeat_count)
          printf("%s\n",line);
      }
    }

    for (n = 0; n < len; n++)
      last_hand[n] = line[offset+n];
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
