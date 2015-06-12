#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: back_to_back (-id) (-same_date) place filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

#define DATE_STR_LEN 10

int main(int argc,char **argv)
{
  int curr_arg;
  bool bId;
  bool bSameDate;
  int back_to_back_place;
  FILE *fptr;
  int line_len;
  int line_no;
  int place;
  int prev_place;
  char session_date[DATE_STR_LEN+1];
  char prev_session_date[DATE_STR_LEN+1];
  int prev_id;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bId = false;
  bSameDate = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-id"))
      bId = true;
    else if (!strcmp(argv[curr_arg],"-same_date"))
      bSameDate = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&back_to_back_place);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  line_no = 0;

  if (bId)
    prev_id = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %s",&place,session_date);

    if (line_no > 1) {
      if ((place == back_to_back_place) && (place == prev_place) &&
        (!bSameDate || !strcmp(prev_session_date,session_date))) {

        if (!bId)
          printf("%s\n",session_date);
        else {
          if (prev_id == -1)
            printf("%s %d\n",session_date,line_no);
          else
            printf("%s %d (%d)\n",session_date,line_no,line_no - prev_id);

          prev_id = line_no;
        }
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
