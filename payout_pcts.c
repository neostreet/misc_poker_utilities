#include <stdio.h>

struct payout {
  int amount;
  int num_places;
};

#define MAX_PAYOUTS 50
static struct payout payouts[MAX_PAYOUTS];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: payout_pcts filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int num_payouts;
  int amount;
  int prize_pool;
  double dwork;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  num_payouts = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&amount);

    if ((line_no == 1) || (amount != payouts[num_payouts].amount)) {
      num_payouts++;

      if (num_payouts == MAX_PAYOUTS) {
        printf("MAX_PAYOUT of %d exceeded\n",MAX_PAYOUTS);
        return 3;
      }

      payouts[num_payouts].amount = amount;
      payouts[num_payouts].num_places = 1;
    }
    else
      payouts[num_payouts].num_places++;
  }

  fclose(fptr);

  num_payouts++;
  prize_pool = 0;

  for (n = 0; n < num_payouts; n++)
    prize_pool += payouts[n].amount * payouts[n].num_places;

  for (n = 0; n < num_payouts; n++) {
    dwork = (double)payouts[n].amount * (double)100 / (double)prize_pool;

    printf("%7.4lf %d %d\n",dwork,payouts[n].amount,payouts[n].num_places);
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
