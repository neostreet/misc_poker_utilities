#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include "str_list.h"

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_PLAYER_NAME_LEN 64
static char player_name[MAX_PLAYER_NAME_LEN+1];

static char usage[] = "usage: fplayer_name (-debug) (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char summary[] = "*** SUMMARY ***";
#define SUMMARY_LEN (sizeof (summary) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_player_name(
  char *line,
  int line_len,
  char *player_name,
  int max_player_name_len
);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bSkipping;
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_len;
  struct info_list players;
  struct info_list_elem *work_elem;
  int ix;
  int retval;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  players.num_elems = 0;

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  num_files = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    bSkipping = false;
    num_files++;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (bSkipping)
        continue;
      else if (!strncmp(line,summary,SUMMARY_LEN))
        bSkipping = true;
      else if (!strncmp(line,"Seat ",5)) {
        retval = get_player_name(line,line_len,player_name,MAX_PLAYER_NAME_LEN);

        if (retval) {
          printf("get_player_name() failed on line %d: %d\n",line_len,retval);
          return 4;
        }

        if (bDebug)
          printf("%s\n",player_name);

        if (member_of_info_list(&players,player_name,&ix)) {
          if (get_info_list_elem(&players,ix,&work_elem))
            work_elem->int1++;
        }
        else
          add_info_list_elem(&players,player_name,1,0,0,true);
      }
    }

    fclose(fptr);
  }

  print_info_list(&players);
  free_info_list(&players);

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

static int get_player_name(
  char *line,
  int line_len,
  char *player_name,
  int max_player_name_len
)
{
  int m;
  int n;

  for (m = 0, n = 8; n < line_len; n++) {
    if ((line[n] == ' ') && (line[n+1] == '('))
      break;

    player_name[m++] = line[n];
  }

  if (n == line_len)
    return 1;

  player_name[m] = 0;

  return 0;
}
