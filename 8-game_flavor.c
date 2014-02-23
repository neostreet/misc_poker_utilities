#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: 8-game_flavor (-debug) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char pokerstars[] = "PokerStars ";
#define POKERSTARS_LEN (sizeof (pokerstars) - 1)
static char eight_game[] = "8-Game (";
#define EIGHT_GAME_LEN (sizeof (eight_game) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
char *get_8_game_flavor(char *line,int line_len,int ix);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  FILE *fptr;
  int line_len;
  int line_no;
  int hand;
  int dbg_hand;
  int dbg;
  int ix;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
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

  line_no = 0;
  hand = 0;
  dbg_hand = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (Contains(true,
      line,line_len,
      pokerstars,POKERSTARS_LEN,
      &ix)) {

      if (Contains(true,
        line,line_len,
        eight_game,EIGHT_GAME_LEN,
        &ix)) {

        hand++;

        if (hand == dbg_hand)
          dbg = 1;

        if (!bDebug)
          printf("%s\n",get_8_game_flavor(line,line_len,ix));
        else
          printf("%5d %5d %s\n",hand,line_no,get_8_game_flavor(line,line_len,ix));
      }
    }
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

static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index)
{
  int m;
  int n;
  int tries;
  char chara;

  tries = line_len - string_len + 1;

  if (tries <= 0)
    return false;

  for (m = 0; m < tries; m++) {
    for (n = 0; n < string_len; n++) {
      chara = line[m + n];

      if (!bCaseSens) {
        if ((chara >= 'A') && (chara <= 'Z'))
          chara += 'a' - 'A';
      }

      if (chara != string[n])
        break;
    }

    if (n == string_len) {
      *index = m;
      return true;
    }
  }

  return false;
}

char *get_8_game_flavor(char *line,int line_len,int ix)
{
  int n;

  for (n = ix + EIGHT_GAME_LEN; n < line_len; n++) {
    if (line[n] == ')') {
      line[n] = 0;
      break;
    }
  }

  return &line[ix + EIGHT_GAME_LEN];
}
