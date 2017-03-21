#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char *poker_styles[] = {
  "Cash game",
  "Sit & Go ",
  "MTT      ",
  "Spin & Go",
  "KO poker ",
  "Power up "
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

struct balance_and_count {
  int curr_balance;
  int count;
};

struct balance_and_count bal_and_count[NUM_POKER_STYLES][NUM_POKER_FLAVORS];

static char usage[] =
"usage: sub_balances filename\n";
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
      bal_and_count[m][n].curr_balance = 0;
      bal_and_count[m][n].count = 0;
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

    bal_and_count[style][flavor].curr_balance += delta;
    bal_and_count[style][flavor].count++;
  }

  fclose(fptr);

  for (m = 0; m < NUM_POKER_STYLES; m++) {
    for (n = 0; n < NUM_POKER_FLAVORS; n++) {
      if (bal_and_count[m][n].curr_balance) {
        printf("%10d %6d   %s   %s\n",
          bal_and_count[m][n].curr_balance,
          bal_and_count[m][n].count,
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
