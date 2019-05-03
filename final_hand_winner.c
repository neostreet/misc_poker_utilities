#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_PLAYER_NAME_LEN 30
static char player_name[MAX_PLAYER_NAME_LEN+1];

static char usage[] =
"usage: final_hand_winner (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char and_won[] = " and won ";
#define AND_WON_LEN (sizeof (and_won) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static int get_player_name(
  char *line,
  int line_len,
  char *player_name,
  int max_player_name_len);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int n;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_no;
  int line_len;
  int ix;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose= true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  getcwd(save_dir,_MAX_PATH);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

  if (feof(fptr0)) {
    fclose(fptr0);
    return 4;
  }

  fclose(fptr0);

  if ((fptr = fopen(filename,"r")) == NULL) {
    printf(couldnt_open,filename);
    return 5;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (Contains(true,
      line,line_len,
      and_won,AND_WON_LEN,
      &ix)) {
      if (!get_player_name(line,line_len,player_name,MAX_PLAYER_NAME_LEN)) {
        if (!bVerbose)
          printf("%s\n",player_name);
        else
          printf("%s %s\n",player_name,save_dir);

        break;
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

static int get_player_name(
  char *line,
  int line_len,
  char *player_name,
  int max_player_name_len)
{
  int m;
  int n;
  int start;
  int end;

  for (n = 0; n < line_len; n++) {
    if (line[n] == ':')
      break;
  }

  if (n == line_len)
    return 1;

  start = n + 2;

  for (n = start; n < line_len; n++) {
    if (line[n] == ' ')
      break;
  }

  if (n == line_len)
    return 2;

  if (n - start > max_player_name_len)
    return 3;

  end = n;

  for (n = start, m = 0; n < end; ) {
    player_name[m++] = line[n++];
  }

  player_name[m] = 0;

  return 0;
}
