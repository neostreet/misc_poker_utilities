#include <stdio.h>
#include <string.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_GAME_NAME_LEN 50
static char game_name[MAX_GAME_NAME_LEN+1];

static char usage[] =
"usage: in_chips (-verbose) (-handed_countcount) (-first_handed_countcount)\n"
"  (-only_8game) (-sum) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char pokerstars[] = "PokerStars";
#define POKERSTARS_LEN (sizeof (pokerstars) - 1)
static char eight_game[] = "8-Game";
#define EIGHT_GAME_LEN (sizeof (eight_game) - 1)
static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static int get_chips(char *line,int ix);
int get_game_name(
  char *line,
  int line_len,
  char *game_name,
  int max_game_name_len);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bHandedCount;
  bool bFirstHandedCount;
  bool bOnly8game;
  bool bSum;
  int handed_count;
  FILE *fptr;
  int line_len;
  int line_no;
  int player_name_ix;
  int player_name_len;
  int ix;
  bool bHaveGameName;
  int retval;
  int chips;
  int total_chips;
  int hand_count;
  int table_count;

  if ((argc < 3) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bHandedCount = false;
  bFirstHandedCount = false;
  bOnly8game = false;
  bSum = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strncmp(argv[curr_arg],"-handed_count",13)) {
      sscanf(&argv[curr_arg][13],"%d",&handed_count);
      bHandedCount = true;
    }
    else if (!strncmp(argv[curr_arg],"-first_handed_count",19)) {
      sscanf(&argv[curr_arg][19],"%d",&handed_count);
      bFirstHandedCount = true;
    }
    else if (!strcmp(argv[curr_arg],"-only_8game"))
      bOnly8game = true;
    else if (!strcmp(argv[curr_arg],"-sum")) {
      bSum = true;
      total_chips = 0;
    }
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (bHandedCount && bFirstHandedCount) {
    printf("can't specify both -handed_countcount and -first_handed_countcount\n");
    return 3;
  }

  if (bFirstHandedCount)
    bHandedCount = true;

  player_name_ix = curr_arg;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr = fopen(argv[curr_arg + 1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg + 1]);
    return 4;
  }

  line_no = 0;
  hand_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (Contains(true,
      line,line_len,
      pokerstars,POKERSTARS_LEN,
      &ix)) {

      bHaveGameName = false;

      if (Contains(true,
        line,line_len,
        eight_game,EIGHT_GAME_LEN,
        &ix)) {

        retval = get_game_name(line,line_len,game_name,MAX_GAME_NAME_LEN);

        if (retval) {
          printf("get_game_name() failed on line %d: %d\n",line_no,retval);
          return 5;
        }

        bHaveGameName = true;
      }
    }
    else if (!strncmp(line,"Table '",7)) {
      hand_count++;
      table_count = 0;

      for ( ; ; ) {
        GetLine(fptr,line,&line_len,MAX_LINE_LEN);

        if (feof(fptr))
          break;

        line_no++;

        if (!strncmp(line,street_marker,STREET_MARKER_LEN))
          break;

        if (!strncmp(line,"Seat ",5)) {
          table_count++;

          if (Contains(true,
            line,line_len,
            argv[player_name_ix],player_name_len,
            &ix)) {

            if (Contains(true,
              line,line_len,
              in_chips,IN_CHIPS_LEN,
              &ix)) {

              chips = get_chips(line,ix);
            }
          }
        }
      }

      if (bSum)
        total_chips += chips;
      else {
        if (!bOnly8game || bHaveGameName) {
          if (!bHandedCount || (table_count == handed_count)) {
            if (!bVerbose)
              printf("%d\n",chips);
            else {
              if (!bHaveGameName)
                printf("%d %d %s hand %d\n",chips,table_count,save_dir,hand_count);
              else
                printf("%d %s %d %s hand %d\n",chips,game_name,table_count,save_dir,hand_count);
            }

            if (bFirstHandedCount)
              break;
          }
        }
      }
    }
  }

  fclose(fptr);

  if (bSum)
    printf("%d\n",total_chips);

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

static int get_chips(char *line,int ix)
{
  int n;
  int chips;

  line[ix] = 0;

  for (n = ix - 1; (n >= 0); n--) {
    if (line[n] == '(')
      break;
  }

  sscanf(&line[n+1],"%d",&chips);

  return chips;
}

int get_game_name(
  char *line,
  int line_len,
  char *game_name,
  int max_game_name_len)
{
  int m;
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == '(')
      break;
  }

  if (n == line_len)
    return 1;

  n++;
  m = 0;

  for ( ; n < line_len; n++) {
    if (line[n] == ')')
      break;

    if (m < max_game_name_len) {
      game_name[m++] = line[n];
    }
    else
      return 2;
  }

  game_name[m] = 0;

  return 0;
}
