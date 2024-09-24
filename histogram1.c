#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: histogram1 (-terse) (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

#define NUM_TIERS 3

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  FILE *fptr;
  int line_len;
  int line_no;
  int places_paid;
  int place;
  int one_third;
  int hist[NUM_TIERS];
  int total;
  int tier;
  double dwork;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
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

  line_no = 0;

  for (n = 0; n < NUM_TIERS; n++)
    hist[n] = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d\t%d",&places_paid,&place);
    one_third = places_paid / 3;
    place--;

    if (place < one_third)
      tier = 0;
    else if (place < 2 * one_third)
      tier = 1;
    else
      tier = 2;

    hist[tier]++;

    if (bVerbose)
      printf("%d (%d %d)\n",tier,places_paid,place);
  }

  total = line_no;

  if (bVerbose)
    putchar(0x0a);

  for (n = 0; n < NUM_TIERS; n++) {
    if (bTerse)
      printf("%d\n",hist[n]);
    else {
      dwork = (double)hist[n] / (double)total;
      printf("%d %lf\n",hist[n],dwork);
    }
  }

  if (!bTerse)
    printf("\n%d\n",total);

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
