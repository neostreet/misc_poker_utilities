#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_SUITS 4
#define NUM_SUIT_COMBOS (NUM_SUITS * NUM_SUITS)
#define NUM_SUIT_COMBOS_COLLAPSED ((NUM_SUIT_COMBOS - NUM_SUITS) / 2 + NUM_SUITS)

/* suit crosstab

  c c
  c d
  c h
  c s
  d c
  d d
  d h
  d s
  h c
  h d
  h h
  h s
  s c
  s d
  s h
  s s
*/

static char *suit_crosstab2[] = {
  "c c",
  "c d",
  "c h",
  "c s",
  "d d",
  "d h",
  "d s",
  "h h",
  "h s",
  "s s"
};

static int suit_combos_collapsed[NUM_SUIT_COMBOS_COLLAPSED];
static int suit_combos_map_to_collapsed[] = {
  0, /* c c */
  1, /* c d */
  2, /* c h */
  3, /* c s */
  1, /* d c */
  4, /* d d */
  5, /* d h */
  6, /* d s */
  2, /* h c */
  5, /* h d */
  7, /* h h */
  8, /* h s */
  3, /* s c */
  6, /* s d */
  8, /* s h */
  9 /* s s */
};

static int breakouts[NUM_SUIT_COMBOS_COLLAPSED];

static char suit_letters[] = "cdhs";
static char bad_suit_in_line[] = "bad suit in line %d: %s\n";

static char usage[] = "usage: session_breakouts filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int num_hands;
  int line_len;
  int line_no;
  int suit_ix1;
  int suit_ix2;
  int map_ix;
  double pct;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  num_hands = 0;

  for (n = 0; n < NUM_SUIT_COMBOS_COLLAPSED; n++)
    breakouts[n] = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    num_hands++;

    for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
      if (line[1] == suit_letters[suit_ix1])
        break;
    }

    if (suit_ix1 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 3;
    }

    for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
      if (line[4] == suit_letters[suit_ix2])
        break;
    }

    if (suit_ix2 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 4;
    }

    map_ix = suit_ix1 * NUM_SUITS + suit_ix2;

    breakouts[suit_combos_map_to_collapsed[map_ix]]++;
  }

  fclose(fptr);

  for (n = 0; n < NUM_SUIT_COMBOS_COLLAPSED; n++) {
    pct = (double)breakouts[n] / (double)num_hands;
    printf("%10d (%6.3lf) %s\n",breakouts[n],pct,suit_crosstab2[n]);
  }

  printf("\n%10d\n",num_hands);

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
