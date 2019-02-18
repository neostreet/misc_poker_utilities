#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define BUY_IN 50000
#define BOUNTY 21250
#define BOUNTY_NO_ENTRY_FEE 25000

static int ko_payouts[] {
  95625,
  57375,
  38250
};

static int ko_payouts_no_entry_fee[] {
  112500,
  67500,
  45000
};

static char usage[] = "usage: fko_delta (-no_entry_fee) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bNoEntryFee;
  FILE *fptr;
  int line_len;
  int line_no;
  int place;
  int num_bounties;
  int delta;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bNoEntryFee = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-no_entry_fee"))
      bNoEntryFee = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(line,"%d %d",&place,&num_bounties);

    if ((place < 1) || (place > 9)) {
      printf("error on line %d: place must be >= 1 and <= 9\n",line_no);
      continue;
    }

    if ((num_bounties < 0) || (num_bounties > 9)) {
      printf("error on line %d: num_bounties must be >= 0 and <= 9\n",line_no);
      continue;
    }

    delta = BUY_IN * -1;

    if ((place >= 1) && (place <= 3))
      delta += (bNoEntryFee ? ko_payouts_no_entry_fee[place - 1] : ko_payouts[place - 1]);

    if (num_bounties)
      delta += num_bounties * (bNoEntryFee ? BOUNTY_NO_ENTRY_FEE : BOUNTY);

    printf("%d\t%d\t%d\n",place,num_bounties,delta);
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
