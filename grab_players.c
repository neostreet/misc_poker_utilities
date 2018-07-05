#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static bool bDebug;

#define MAX_PLAYER_NAME_LEN 40
#define MAX_PLAYERS 9

static char player_names[MAX_PLAYERS][MAX_PLAYER_NAME_LEN+1];

static char save_dir[_MAX_PATH];

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];
static char max_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: grab_players (-debug) (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static int grab_player_name(char *line,int line_len,char *player_name);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  FILE *fptr0;
  int filenamelen;
  int hand_no;
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;
  int num_players;
  int retval;

  if ((argc < 2) && (argc > 4)) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  bDebug = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  hand_no = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filenamelen,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    hand_no++;
    printf("%s\n",filename);

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    num_players = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (!strncmp(line,"Seat ",5)) {
        retval = grab_player_name(line,line_len,&player_names[num_players++][0]);

        if (retval) {
          printf("grab_player_name() failed: %d\n",retval);
          return 4;
        }
      }
      else if (Contains(true,
        line,line_len,
        street_marker,STREET_MARKER_LEN,
        &ix)) {

        break;
      }
    }

    fclose(fptr);

    for (n = 0; n < num_players; n++)
      printf("  %s\n",player_names[n]);
  }

  fclose(fptr0);

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

static int grab_player_name(char *line,int line_len,char *player_name)
{
  int ix;

  if (!Contains(true,
    line,line_len,
    (char*)" (",2,
    &ix)) {

    return 1;
  }

  if (ix - 8 > MAX_PLAYER_NAME_LEN)
    return 2;

  if (bDebug)
    printf("%d\n",ix - 8);

  memcpy(player_name,&line[8],ix - 8);
  player_name[ix - 8] =  0;

  return 0;
}
