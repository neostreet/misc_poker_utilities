#include <stdio.h>

enum Suit {
  CLUBS,
  DIAMONDS,
  HEARTS,
  SPADES,
  NUM_SUITS
};

static char suit_chars[] = "cdhs";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: aggreg_suits offset filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int offset;
  FILE *fptr;
  int line_len;
  int line_no;
  int suit_counts[NUM_SUITS];
  int total;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&offset);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  for (n = 0; n < NUM_SUITS; n++)
    suit_counts[n] = 0;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    for (n = 0; n < NUM_SUITS; n++) {
      if (line[offset] == suit_chars[n])
        break;
    }

    if (n == NUM_SUITS) {
      printf("invalid suit on line %d\n",line_no);
      return 3;
    }

    suit_counts[n]++;
  }

  fclose(fptr);

  total = 0;

  for (n = 0; n < NUM_SUITS; n++) {
    printf("%c %5d\n",suit_chars[n],suit_counts[n]);
    total += suit_counts[n];
  }

  printf("\n  %5d\n",total);

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
