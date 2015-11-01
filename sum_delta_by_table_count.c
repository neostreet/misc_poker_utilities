#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_PLAYERS 9

struct delta_info {
  int sum_pos_delta;
  int sum_neg_delta;
  int count;
};

static char usage[] =
"usage: sum_delta_by_table_count (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int bVerbose;
  FILE *fptr;
  int line_len;
  int line_no;
  struct delta_info sum_delta[MAX_PLAYERS - 2];
  int delta;
  int table_count;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
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

  for (n = 0; n < MAX_PLAYERS - 2; n++) {
    sum_delta[n].sum_pos_delta = 0;
    sum_delta[n].sum_neg_delta = 0;
    sum_delta[n].count = 0;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(line,"%d %d",&delta,&table_count);

    if (table_count < 2) {
      printf("line %d: table count (%d) < 2\n",
        line_no,table_count);
      return 4;
    }

    if (table_count > MAX_PLAYERS) {
      printf("line %d: table count (%d) > MAX_PLAYERS (%d)\n",
        line_no,table_count,MAX_PLAYERS);
      return 5;
    }

    table_count -= 2;

    if (delta > 0)
      sum_delta[table_count].sum_pos_delta += delta;
    else if (delta < 0)
      sum_delta[table_count].sum_neg_delta += delta;

    sum_delta[table_count].count++;
  }

  fclose(fptr);

  for (n = MAX_PLAYERS - 3; (n >= 0); n--) {
    delta = sum_delta[n].sum_pos_delta + sum_delta[n].sum_neg_delta;

    if (delta) {
      if (!bVerbose)
        printf("%d %10d %5d\n",n + 2,delta,sum_delta[n].count);
      else {
        printf("%d %10d %10d %10d %5d\n",n + 2,delta,
          sum_delta[n].sum_pos_delta,sum_delta[n].sum_neg_delta,
          sum_delta[n].count);
      }
    }
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
