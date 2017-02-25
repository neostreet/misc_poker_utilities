#include <stdio.h>

struct payout {
  int amount;
  int num_places;
};

#define MAX_PAYOUTS 50
static struct payout payouts[MAX_PAYOUTS];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: final_table_pct filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int num_payouts;
  int prize_pool;
  int paid_places;
  int final_table_amount;
  double dwork;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  num_payouts = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (num_payouts == MAX_PAYOUTS) {
      printf("MAX_PAYOUT of %d exceeded\n",MAX_PAYOUTS);
      return 3;
    }

    sscanf(line,"%d %d",
      &payouts[num_payouts].amount,
      &payouts[num_payouts].num_places);
    num_payouts++;
  }

  fclose(fptr);

  prize_pool = 0;
  paid_places = 0;

  for (n = 0; n < num_payouts; n++) {
    prize_pool += payouts[n].amount * payouts[n].num_places;
    paid_places += payouts[n].num_places;
  }

  final_table_amount = 0;

  for (n = 0; n < num_payouts; n++) {
    if (payouts[n].num_places == 1)
      final_table_amount += payouts[n].amount;
    else
      break;
  }

  dwork = (double)final_table_amount / (double)prize_pool;

  printf("%8d %8d %7.4lf\n",final_table_amount,prize_pool,dwork);

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
