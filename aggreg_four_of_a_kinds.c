#include <stdio.h>

static char rank_chars[] = "23456789TJQKA";
static char *rank_strings[] = {
  "twos",
  "threes",
  "fours",
  "fives",
  "sixes",
  "sevens",
  "eights",
  "nines",
  "tens",
  "jacks",
  "queens",
  "kings",
  "aces"
};

#define NUM_CARDS_IN_SUIT 13

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: aggreg_four_of_a_kinds filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  char four_of_a_kinds[NUM_CARDS_IN_SUIT];

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  for (n = 0; n < NUM_CARDS_IN_SUIT; n++)
    four_of_a_kinds[n] = 0;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (line_len < 33) {
      printf("line %d is too short\n",line_no);
      return 3;
    }

    for (n = 0; n < NUM_CARDS_IN_SUIT; n++) {
      if (line[32] == rank_chars[n])
        break;
    }

    if (n == NUM_CARDS_IN_SUIT) {
      printf("line %d has a bad rank char: %c\n",line_no,line[32]);
      return 4;
    }

    four_of_a_kinds[n]++;
  }

  fclose(fptr);

  for (n = NUM_CARDS_IN_SUIT - 1; (n >= 0); n--)
    printf("%3d quad %s\n",four_of_a_kinds[n],rank_strings[n]);

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
