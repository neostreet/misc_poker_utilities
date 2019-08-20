#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

enum actions {
  ACTION_FOLD,
  ACTION_CHECK,
  ACTION_CALL,
  ACTION_BET,
  ACTION_RAISE,
  NUM_ACTIONS
};

char *action_strings[] = {
  "fold ",
  "check",
  "call ",
  "bet  ",
  "raise"
};

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fdecpcts (-debug) (-verbose) (-actionaction)\n"
"  player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char summary[] = "*** SUMMARY ***";
#define SUMMARY_LEN (sizeof (summary) - 1)
static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)
static char folds[] = " folds ";
#define FOLDS_LEN (sizeof (folds) - 1)
static char checks[] = " checks ";
#define CHECKS_LEN (sizeof (checks) - 1)
static char calls[] = " calls ";
#define CALLS_LEN (sizeof (calls) - 1)
static char bets[] = " bets ";
#define BETS_LEN (sizeof (bets) - 1)
static char raises[] = " raises ";
#define RAISES_LEN (sizeof (raises) - 1)

static char fmt1[] = "%6d %6d %lf\n";
static char fmt2[] = "%6d %6d %lf %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  int action;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  int ix;
  int file_no;
  int dbg_file_no;
  int dbg;
  int tot_actions[NUM_ACTIONS];
  int tot_tot_actions;
  int bet_possible_actions;
  int bet_not_possible_actions;
  double dwork;

  if ((argc < 3) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  action = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strncmp(argv[curr_arg],"-action",7)) {
      for (action = ACTION_FOLD; action < NUM_ACTIONS; action++) {
        if(!strcmp(&argv[curr_arg][7],action_strings[action]))
          break;
      }

      if (action == NUM_ACTIONS) {
        printf("invalid action: %s\n",&argv[curr_arg][7]);
        return 2;
      }
    }
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 3;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if (bVerbose)
    getcwd(save_dir,_MAX_PATH);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  file_no = 0;
  dbg_file_no = -1;

  for (n = 0; n < NUM_ACTIONS; n++)
    tot_actions[n] = 0;

  bet_possible_actions = 0;
  bet_not_possible_actions = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    file_no++;

    if (dbg_file_no == file_no)
      dbg = 1;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (Contains(true,
          line,line_len,
          folds,FOLDS_LEN,
          &ix)) {

          tot_actions[ACTION_FOLD]++;
        }
        else if (Contains(true,
          line,line_len,
          checks,CHECKS_LEN,
          &ix)) {

          tot_actions[ACTION_CHECK]++;
        }
        else if (Contains(true,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {

          tot_actions[ACTION_CALL]++;
        }
        else if (Contains(true,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {

          tot_actions[ACTION_BET]++;
        }
        else if (Contains(true,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {

          tot_actions[ACTION_RAISE]++;
        }
      }
    }

    fclose(fptr);
  }

  fclose(fptr0);

  tot_tot_actions = 0;

  for (n = 0; n < NUM_ACTIONS; n++) {
    tot_tot_actions += tot_actions[n];
  }

  if (action == -1) {
    for (n = 0; n < NUM_ACTIONS; n++) {
      dwork = (double)tot_actions[n] / (double)tot_tot_actions;

      if (!bVerbose) {
        printf("%6.4lf %s %7d\n",dwork,action_strings[n],
          tot_actions[n]);
      }
      else {
        printf("%6.4lf %s %7d %7d %s\n",dwork,action_strings[n],
          tot_actions[n],tot_tot_actions,save_dir);
      }
    }
  }
  else {
    dwork = (double)tot_actions[action] / (double)tot_tot_actions;

    if (!bVerbose) {
      printf("%6.4lf %s %7d\n",dwork,action_strings[action],
        tot_actions[action]);
    }
    else {
      printf("%6.4lf %s %7d %7d %s\n",dwork,action_strings[action],
        tot_actions[action],tot_tot_actions,save_dir);
    }
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
