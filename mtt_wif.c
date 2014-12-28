#include <stdio.h>

#define MAX_PAID_PLACES 24
static int payouts[MAX_PAID_PLACES];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: mtt_wif buy_in entry_fee pay_table places\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int num_paid_places;
  int line_len;
  int buy_in;
  int entry_fee;
  int profit;
  int place;

  if (argc != 5) {
    printf(usage);
    return 1;
  }
  sscanf(argv[1],"%d",&buy_in);
  sscanf(argv[2],"%d",&entry_fee);

  if ((fptr = fopen(argv[3],"r")) == NULL) {
    printf(couldnt_open,argv[3]);
    return 2;
  }

  num_paid_places = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (num_paid_places == MAX_PAID_PLACES) {
      printf("MAX_PAID_PLACES of %d exceeded\n",MAX_PAID_PLACES);
      return 3;
    }

    sscanf(line,"%d",&payouts[num_paid_places]);

    num_paid_places++;
  }

  fclose(fptr);

  if ((fptr = fopen(argv[4],"r")) == NULL) {
    printf(couldnt_open,argv[4]);
    return 2;
  }

  profit = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    profit -= (buy_in + entry_fee);

    sscanf(line,"%d",&place);

    if ((place >= 1) && (place <= num_paid_places)) {
      place -= 1;
      profit += payouts[place];
    }
  }

  printf("%d\n",profit);

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
