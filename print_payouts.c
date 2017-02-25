#include <stdio.h>
#include <string.h>

struct payout {
  int amount;
  int num_places;
};

#define MAX_PAYOUTS 50
static struct payout payouts[MAX_PAYOUTS];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: print_payouts (-unique) (-reverse) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bUnique;
  bool bReverse;
  FILE *fptr;
  int line_len;
  int num_payouts;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bUnique = false;
  bReverse = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-unique"))
      bUnique = true;
    else if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = true;
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

  num_payouts = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (num_payouts == MAX_PAYOUTS) {
      printf("MAX_PAYOUT of %d exceeded\n",MAX_PAYOUTS);
      return 4;
    }

    sscanf(line,"%d %d",
      &payouts[num_payouts].amount,
      &payouts[num_payouts].num_places);
    num_payouts++;
  }

  fclose(fptr);

  if (!bReverse) {
    for (n = num_payouts - 1; n >= 0; n--) {
      if (!bUnique) {
        for (m = payouts[n].num_places - 1; m >= 0; m--)
          printf("%d\n",payouts[n].amount);
      }
      else
        printf("%d\n",payouts[n].amount);
    }
  }
  else {
    for (n = 0; n < num_payouts; n++) {
      if (!bUnique) {
        for (m = 0; m < payouts[n].num_places; m++)
          printf("%d\n",payouts[n].amount);
      }
      else
        printf("%d\n",payouts[n].amount);
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
