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
static char save_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: final_table_rung (-verbose) rung player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int rung;
  int total_chips;
  int prev_total_chips;
  int num_players;
  int prev_num_players;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filenamelen;
  FILE *fptr;
  int file_no;
  int line_len;
  int line_no;
  int ix;
  int player_chips;
  int prev_player_chips;
  int work;
  double dwork;
  double save_dwork;

  if ((argc < 4) && (argc > 5)) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 3) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg++],"%d",&rung);
  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  file_no = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filenamelen,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    file_no++;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    player_chips = 0;

    total_chips = 0;
    num_players = 0;

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

        total_chips += work;
        num_players++;

        if (Contains(true,
          line,line_len,
          argv[player_name_ix],player_name_len,
          &ix)) {
          player_chips = work;
        }
      }
      else if (num_players)
        break;
    }

    fclose(fptr);

    if (file_no == 1)
      prev_total_chips = total_chips;
    else if (total_chips != prev_total_chips)
      break;

    if ((file_no > 1) && (prev_num_players == rung) && (num_players != rung)) {
      dwork = (double)prev_player_chips / (double)total_chips;

      if (!bVerbose)
        printf("%7.4lf %s/%s\n",dwork,save_dir,save_filename);
      else {
        printf("%7.4lf (%d %10d %10d) %s/%s\n",dwork,
          prev_num_players,prev_player_chips,total_chips,save_dir,save_filename);
      }

      break;
    }

    strcpy(save_filename,filename);
    prev_player_chips = player_chips;
    prev_num_players = num_players;
  }

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
