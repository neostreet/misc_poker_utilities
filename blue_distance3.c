#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char *poker_styles[] = {
  "Cash game",
  "Sit & Go ",
  "MTT      ",
  "Spin & Go",
  "KO poker "
};

#define NUM_POKER_STYLES (sizeof poker_styles / sizeof (char *))

static char *poker_flavors[] = {
  "Pot Limit Hold'em",
  "Pot Limit Omaha",
  "Limit 7 Card Stud",
  "No Limit Hold'em",
  "Limit 5 Card Draw",
  "No Limit Triple Draw 2-7 Lowball",
  "Pot Limit Triple Draw 2-7 Lowball",
  "Limit Hold'em",
  "Limit Triple Draw 2-7 Lowball",
  "8-Game",
  "HORSE"
};

#define NUM_POKER_FLAVORS (sizeof poker_flavors / sizeof (char *))

static int curr_balance[NUM_POKER_STYLES][NUM_POKER_FLAVORS];
static int max_balance[NUM_POKER_STYLES][NUM_POKER_FLAVORS];

static char usage[] = "usage: blue_distance3 filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int delta;
  int style;
  int flavor;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  for (m = 0; m < NUM_POKER_STYLES; m++) {
    for (n = 0; n < NUM_POKER_FLAVORS; n++) {
      curr_balance[m][n] = 0;
      max_balance[m][n] = 0;
    }
  }

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(line,"%d %d %d",&delta,&style,&flavor);

    if ((style < 0) || (style >= NUM_POKER_STYLES)) {
      printf("invalid style %d on line %d\n",style,line_no);
      return 3;
    }

    if ((flavor < 0) || (flavor >= NUM_POKER_FLAVORS)) {
      printf("invalid flavor %d on line %d\n",flavor,line_no);
      return 4;
    }

    curr_balance[style][flavor] += delta;

    if (curr_balance[style][flavor] > max_balance[style][flavor])
      max_balance[style][flavor] = curr_balance[style][flavor];
  }

  fclose(fptr);

  for (m = 0; m < NUM_POKER_STYLES; m++) {
    for (n = 0; n < NUM_POKER_FLAVORS; n++) {
      if (curr_balance[m][n]) {
        printf("%10d   %s   %s\n",max_balance[m][n] - curr_balance[m][n],
          poker_styles[m],poker_flavors[n]);
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
