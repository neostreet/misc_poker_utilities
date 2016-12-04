#include <stdio.h>

struct payout {
  int amount;
  int num_places;
};

#define MAX_PAYOUTS 50
static struct payout payouts[MAX_PAYOUTS];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: print_payouts filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  FILE *fptr;
  int line_len;
  int num_payouts;

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

  for (n = num_payouts - 1; n >= 0; n--) {
    for (m = payouts[n].num_places - 1; m >= 0; m--)
      printf("%d\n",payouts[n].amount);
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
