#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: filter_premium_hands filename\n";
static char couldnt_open[] = "couldn't open %s\n";

char *premium_hand_abbrevs[] = {
  "AA",
  "KK",
  "QQ",
  "AKs",
  "JJ",
  "TT",
  "AQs",
  "AKo",
  "AJs",
  "KQs"
};
#define NUM_PREMIUM_HAND_ABBREVS (sizeof premium_hand_abbrevs / sizeof (char *))

static int premium_hand_abbrev_lens[NUM_PREMIUM_HAND_ABBREVS];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static bool filter_premium_hands(char *line,int line_len);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int line_no;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  for (n = 0; n < NUM_PREMIUM_HAND_ABBREVS; n++)
    premium_hand_abbrev_lens[n] = strlen(premium_hand_abbrevs[n]);

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (filter_premium_hands(line,line_len))
      printf("%s\n",line);
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

static bool filter_premium_hands(char *line,int line_len)
{
  int n;

  for (n = 0; n < NUM_PREMIUM_HAND_ABBREVS; n++) {
    premium_hand_abbrev_lens[n] = strlen(premium_hand_abbrevs[n]);

    if (!strncmp(line,premium_hand_abbrevs[n],premium_hand_abbrev_lens[n]))
      return true;
  }

  return false;
}
