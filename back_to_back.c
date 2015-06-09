#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: back_to_back place filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

#define DATE_STR_LEN 10

int main(int argc,char **argv)
{
  int back_to_back_place;
  FILE *fptr;
  int line_len;
  int line_no;
  int place;
  int prev_place;
  char session_date[DATE_STR_LEN+1];
  char prev_session_date[DATE_STR_LEN+1];

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&back_to_back_place);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %s",&place,session_date);

    if (line_no > 1) {
      if ((place == back_to_back_place) && (place == prev_place) &&
        !strcmp(prev_session_date,session_date)) {
          printf("%s\n",session_date);
      }
    }

    prev_place = place;
    strcpy(prev_session_date,session_date);
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
