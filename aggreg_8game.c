#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 4096
static char line[MAX_LINE_LEN];

#define MAX_GAME_NAME_LEN 50
static char game_name[MAX_GAME_NAME_LEN+1];

static char usage[] = "usage: aggreg_8game filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char *eight_game_names[] = {
  "Triple Draw 2-7 Lowball Limit",
  "Hold'em Limit",
  "Omaha Hi/Lo Limit",
  "Razz Limit",
  "7 Card Stud Limit",
  "7 Card Stud Hi/Lo Limit",
  "Hold'em No Limit",
  "Omaha Pot Limit"
};
#define NUM_8GAME_GAMES (sizeof eight_game_names / sizeof (char *))

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
int get_delta_and_8game_name(
  char *line,
  int line_len,
  int *delta_ptr,
  char *game_name,
  int max_game_name_len);
int get_8game_ix(char *game_name,int *ix);

int main(int argc,char **argv)
{
  int n;
  int retval;
  int delta;
  int ix;
  FILE *fptr;
  int line_len;
  int line_no;
  int balances[NUM_8GAME_GAMES];

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  for (n = 0; n < NUM_8GAME_GAMES; n++)
    balances[n] = 0;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    retval = get_delta_and_8game_name(line,line_len,&delta,game_name,MAX_GAME_NAME_LEN);

    if (retval) {
      printf("get_delta_and_8game_name() failed on line %d: %d\n",line_no,retval);
      return 3;
    }

    retval = get_8game_ix(game_name,&ix);

    if (retval) {
      printf("get_8game_ix() failed on line %d: %d\n",line_no,retval);
      return 4;
    }

    balances[ix] += delta;
  }

  fclose(fptr);

  for (n = 0; n < NUM_8GAME_GAMES; n++)
    printf("%10d %s\n",balances[n],eight_game_names[n]);

  return 0;
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

int get_delta_and_8game_name(
  char *line,
  int line_len,
  int *delta_ptr,
  char *game_name,
  int max_game_name_len)
{
  int n;
  int ix;

  sscanf(line,"%d",delta_ptr);

  if (!Contains(true,
    line,line_len,
    " /cygdrive",10,
    &ix)) {

    return 1;
  }

  for (n = 40; n < ix; n++)
    game_name[n - 40] = line[n];

  game_name[n - 40] = 0;

  return 0;
}

int get_8game_ix(char *game_name,int *ix)
{
  int n;

  for (n = 0; n < NUM_8GAME_GAMES; n++) {
    if (!strcmp(game_name,eight_game_names[n])) {
      *ix = n;
      return 0;
    }
  }

  return 1;
}
