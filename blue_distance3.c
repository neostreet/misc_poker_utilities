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

struct balances_and_count {
  int curr_balance;
  int max_balance;
  int count;
};

struct balances_and_count bals_and_count[NUM_POKER_STYLES][NUM_POKER_FLAVORS];

static char usage[] =
"usage: blue_distance3 (-count_gen) (-show_counts) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  int count_ge;
  bool bShowCounts;
  FILE *fptr;
  int line_len;
  int line_no;
  int delta;
  int style;
  int flavor;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  count_ge = 0;
  bShowCounts = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strncmp(argv[curr_arg],"-count_ge",9))
      sscanf(&argv[curr_arg][9],"%d",&count_ge);
    else if (!strcmp(argv[curr_arg],"-show_counts"))
      bShowCounts = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 2;
  }

  line_no = 0;

  for (m = 0; m < NUM_POKER_STYLES; m++) {
    for (n = 0; n < NUM_POKER_FLAVORS; n++) {
      bals_and_count[m][n].curr_balance = 0;
      bals_and_count[m][n].max_balance = 0;
      bals_and_count[m][n].count = 0;
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

    bals_and_count[style][flavor].curr_balance += delta;

    if (bals_and_count[style][flavor].curr_balance > bals_and_count[style][flavor].max_balance)
      bals_and_count[style][flavor].max_balance = bals_and_count[style][flavor].curr_balance;

    bals_and_count[style][flavor].count++;
  }

  fclose(fptr);

  for (m = 0; m < NUM_POKER_STYLES; m++) {
    for (n = 0; n < NUM_POKER_FLAVORS; n++) {
      if (bals_and_count[m][n].curr_balance) {
        if (!count_ge || (bals_and_count[m][n].count >= count_ge)) {
          if (!bShowCounts) {
            printf("%10d   %s   %s\n",
              bals_and_count[m][n].max_balance - bals_and_count[m][n].curr_balance,
              poker_styles[m],poker_flavors[n]);
          }
          else {
            printf("%10d %6d   %s   %s\n",
              bals_and_count[m][n].max_balance - bals_and_count[m][n].curr_balance,
              bals_and_count[m][n].count,
              poker_styles[m],poker_flavors[n]);
          }
        }
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
