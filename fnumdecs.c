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

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fnumdecs (-debug) (-verbose) (-terse) (-action) (-zero) (-folded)\n"
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
static char bets[] = " bets ";
#define BETS_LEN (sizeof (bets) - 1)
static char calls[] = " calls ";
#define CALLS_LEN (sizeof (calls) - 1)
static char raises[] = " raises ";
#define RAISES_LEN (sizeof (raises) - 1)
static char checks[] = " checks ";
#define CHECKS_LEN (sizeof (checks) - 1)

static char fmt1[] = "%6d %6d %lf\n";
static char fmt2[] = "%6d %6d %lf %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bTerse;
  int action;
  int zero;
  int folded;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;
  int file_no;
  int dbg_file_no;
  int dbg;
  char hole_cards[6];
  int tot_num_hands;
  int num_hands;
  int tot_numdecs;
  int numdecs;
  int tot_numfolds;
  int numfolds;
  int tot_numchecks;
  int numchecks;
  int tot_numcalls;
  int numcalls;
  int tot_numbets;
  int numbets;
  int tot_numraises;
  int numraises;
  int tot_action_numdecs;
  int action_numdecs;
  int tot_zero_numdecs;
  int zero_numdecs;
  double dwork;

  if ((argc < 3) || (argc > 9)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bTerse = false;
  action = 0;
  zero = 0;
  folded = 0;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-action"))
      action = 1;
    else if (!strcmp(argv[curr_arg],"-zero"))
      zero = 1;
    else if (!strcmp(argv[curr_arg],"-folded"))
      folded = 1;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (action + zero + folded > 1) {
    printf("can only specify one of -action, -zero, and -folded\n");
    return 3;
  }

  if (bVerbose && bTerse) {
    printf("can't specify both -verbose and -terse\n");
    return 4;
  }

  if (bDebug)
    getcwd(save_dir,_MAX_PATH);

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 5;
  }

  file_no = 0;
  dbg_file_no = -1;

  hole_cards[5] = 0;

  tot_num_hands = 0;
  tot_numdecs = 0;
  tot_numfolds = 0;
  tot_numchecks = 0;
  tot_numcalls = 0;
  tot_numbets = 0;
  tot_numraises = 0;

  if (action)
    tot_action_numdecs = 0;
  else if (zero)
    tot_zero_numdecs = 0;

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
    num_hands = 0;
    numdecs = 0;
    numfolds = 0;
    numchecks = 0;
    numcalls = 0;
    numbets = 0;
    numraises = 0;

    if (action)
      action_numdecs = 0;
    else if (zero)
      zero_numdecs = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (!strncmp(line,dealt_to,DEALT_TO_LEN)) {
          for (n = 0; n < line_len; n++) {
            if (line[n] == '[')
              break;
          }

          if (n < line_len) {
            n++;

            for (m = n; m < line_len; m++) {
              if (line[m] == ']')
                break;
            }

            if (m < line_len) {
              for (p = 0; p < 5; p++)
                hole_cards[p] = line[n+p];
            }
          }
        }
        else if (Contains(true,
          line,line_len,
          folds,FOLDS_LEN,
          &ix)) {

          numfolds++;
          numdecs++;

          if (zero)
            zero_numdecs++;
        }
        else if (Contains(true,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {

          numbets++;
          numdecs++;

          if (action)
            action_numdecs++;
        }
        else if (Contains(true,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {

          numcalls++;
          numdecs++;

          if (action)
            action_numdecs++;
        }
        else if (Contains(true,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {

          numraises++;
          numdecs++;

          if (action)
            action_numdecs++;
        }
        else if (Contains(true,
          line,line_len,
          checks,CHECKS_LEN,
          &ix)) {

          numchecks++;
          numdecs++;

          if (zero)
            zero_numdecs++;
        }
      }
      else {
        if (!strncmp(line,summary,SUMMARY_LEN))
          num_hands++;
      }
    }

    fclose(fptr);

    if (bVerbose) {
      dwork = (double)numdecs / (double)num_hands;

      printf("%6d %6d %5.2lf fld %4d chk %4d call %4d bet %4d rse %4d %s\n",
         numdecs,num_hands,dwork,
         numfolds,numchecks,numcalls,numbets,numraises,
         filename);
    }

    tot_num_hands += num_hands;
    tot_numdecs += numdecs;
    tot_numfolds += numfolds;
    tot_numchecks += numchecks;
    tot_numcalls += numcalls;
    tot_numbets += numbets;
    tot_numraises += numraises;

    if (action)
      tot_action_numdecs += action_numdecs;
    else if (zero)
      tot_zero_numdecs += zero_numdecs;
  }

  fclose(fptr0);

  if (action) {
    dwork = (double)tot_action_numdecs / (double)tot_numdecs;

    if (!bDebug)
      printf(fmt1,tot_action_numdecs,tot_numdecs,dwork);
    else
      printf(fmt2,tot_action_numdecs,tot_numdecs,dwork,save_dir);
  }
  else if (zero) {
    dwork = (double)tot_zero_numdecs / (double)tot_numdecs;

    if (!bDebug)
      printf(fmt1,tot_zero_numdecs,tot_numdecs,dwork);
    else
      printf(fmt2,tot_zero_numdecs,tot_numdecs,dwork,save_dir);
  }
  else if (folded) {
    dwork = (double)tot_numfolds / (double)tot_numdecs;

    if (!bDebug)
      printf(fmt1,tot_numfolds,tot_numdecs,dwork);
    else
      printf(fmt2,tot_numfolds,tot_numdecs,dwork,save_dir);
  }
  else {
    dwork = (double)tot_numdecs / (double)tot_num_hands;

    if (!bVerbose) {
      if (bTerse) {
        printf("%6d %6d %5.2lf fld %4d chk %4d call %4d bet %4d rse %4d\n",
          tot_numdecs,tot_num_hands,dwork,
          tot_numfolds,tot_numchecks,tot_numcalls,tot_numbets,tot_numraises);
      }
      else if (!bDebug)
        printf(fmt1,tot_numdecs,tot_num_hands,dwork);
      else
        printf(fmt2,tot_numdecs,tot_num_hands,dwork,save_dir);
    }
    else {
      printf("%6d %6d %5.2lf fld %4d chk %4d call %4d %4d bet rse %4d\n",
        tot_numdecs,tot_num_hands,dwork,
        tot_numfolds,tot_numchecks,tot_numcalls,tot_numbets,tot_numraises);
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
