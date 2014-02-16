#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: 8-game_flavor filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char eight_game[] = "8-Game (";
#define EIGHT_GAME_LEN (sizeof (eight_game) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
char *get_8_game_flavor(char *line,int line_len,int ix);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (!strncmp(line,"PokerStars ",11)) {
      if (Contains(true,
        line,line_len,
        eight_game,EIGHT_GAME_LEN,
        &ix)) {

        printf("%s\n",get_8_game_flavor(line,line_len,ix));
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
