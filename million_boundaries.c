#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: million_boundaries (-debug) (-all) (-all_up) (-all_down) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bAll;
  bool bUp;
  bool bDown;
  bool bCrossed;
  FILE *fptr;
  int line_len;
  int line_no;
  int boundary;
  int last_boundary_ix;
  char date_str[20];
  int starting_balance;
  int ending_balance;
  int starting_million;
  int ending_million;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bAll = false;
  bUp = false;
  bDown = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-all"))
      bAll = true;
    else if (!strcmp(argv[curr_arg],"-all_up")) {
      bAll = true;
      bUp = true;
    }
    else if (!strcmp(argv[curr_arg],"-all_down")) {
      bAll = true;
      bDown = true;
    }
    else
      break;
  }

  if (bUp && bDown) {
    printf("can't specify both -all_up and -all_down\n");
    return 2;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;

  if (!bAll)
    boundary = 1000000;

  if (bDebug)
    last_boundary_ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%s\t%d\t%d",&date_str[0],&starting_balance,&ending_balance);

    if (!bAll) {
      if ((starting_balance < boundary) && (boundary < ending_balance))
        bCrossed = true;
      else
        bCrossed = false;
    }
    else {
      bCrossed = false;

      starting_million = starting_balance / 1000000;
      ending_million = ending_balance / 1000000;

      if (starting_million != ending_million) {
        if (!bUp && !bDown)
          bCrossed = true;
        else if (bUp && (starting_million < ending_million))
          bCrossed = true;
        else if (bDown && (starting_million > ending_million))
          bCrossed = true;
      }
    }

    if (bCrossed) {
      if (!bDebug)
        printf("%s\t%d\t%d\n",date_str,starting_balance,ending_balance);
      else {
        printf("%s\t%d\t%d (%d %d %d)\n",date_str,starting_balance,ending_balance,
          last_boundary_ix,line_no,line_no - last_boundary_ix);
        last_boundary_ix = line_no;
      }

      if (!bAll)
        boundary += 1000000;
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
