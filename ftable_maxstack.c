#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];
static char max_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_PLAYER_NAME_LEN 30
static char player_name[MAX_PLAYER_NAME_LEN+1];

static char usage[] =
"usage: ftable_maxstack (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)

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
  FILE *fptr0;
  int filenamelen;
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;
  int table_max_stack;
  int max_table_max_stack;
  int work;

  if ((argc != 2) && (argc != 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
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

  if (!bVerbose)
    max_table_max_stack = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filenamelen,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    table_max_stack = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
        line,line_len,
        in_chips,IN_CHIPS_LEN,
        &ix)) {

        line[ix] = 0;

        for (ix--; (ix >= 0) && (line[ix] != '('); ix--)
          ;

        sscanf(&line[ix+1],"%d",&work);

        if (work > table_max_stack) {
          table_max_stack = work;

          get_player_name(line,line_len,player_name,MAX_PLAYER_NAME_LEN);
        }
      }
    }

    fclose(fptr);

    if (bVerbose)
      printf("%10d %s %s/%s\n",table_max_stack,player_name,save_dir,filename);
    else if (table_max_stack > max_table_max_stack) {
      strcpy(max_filename,filename);
      max_table_max_stack = table_max_stack;
    }
  }

  if (!bVerbose)
    printf("%10d %s\n",max_table_max_stack,max_filename);

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
  int n;

  for (n = 0; n < max_player_name_len; n++) {
    if ((8 + n >= line_len) || (line[8+n] == '('))
      break;

    player_name[n] = line[8+n];
  }

  player_name[n] = 0;
}
