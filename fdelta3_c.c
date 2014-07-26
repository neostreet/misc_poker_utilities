#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fdelta3_c (-terse) (-stud) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char summary[] = "*** SUMMARY ***";
#define SUMMARY_LEN (sizeof (summary) - 1)
static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)
static char posts_the_ante[] = " posts the ante ";
#define POSTS_THE_ANTE_LEN (sizeof (posts_the_ante) - 1)
static char brings_in_for[] = " brings in for ";
#define BRINGS_IN_FOR_LEN (sizeof (brings_in_for) - 1)
static char posts[] = " posts ";
#define POSTS_LEN (sizeof (posts) - 1)
static char folds[] = " folds ";
#define FOLDS_LEN (sizeof (folds) - 1)
static char bets[] = " bets ";
#define BETS_LEN (sizeof (bets) - 1)
static char calls[] = " calls ";
#define CALLS_LEN (sizeof (calls) - 1)
static char raises[] = " raises ";
#define RAISES_LEN (sizeof (raises) - 1)
static char uncalled_bet[] = "Uncalled bet (";
#define UNCALLED_BET_LEN (sizeof (uncalled_bet) - 1)
static char collected[] = " collected ";
#define COLLECTED_LEN (sizeof (collected) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
int get_work_amount(char *line,int line_len);
static int get_big_blind(
  char *line,
  int line_len,
  int *big_blind_ptr
);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bTerse;
  bool bStud;
  bool bFolded;
  bool bSkipping;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int dbg_line_no;
  int ix;
  int street;
  int num_street_markers;
  int max_streets;
  int starting_balance;
  int ante;
  int bring_in;
  int spent_this_street;
  int spent_this_hand;
  int end_ix;
  int uncalled_bet_amount;
  int collected_from_pot;
  int collected_from_pot_count;
  int ending_balance;
  int delta;
  int file_no;
  int dbg_file_no;
  int num_hands;
  int dbg;
  int work;
  double dwork1;
  double dwork2;
  double sum_deltas_dwork;
  int sum_deltas;
  int sum_positive_deltas;
  int sum_negative_deltas;
  int sum_absolute_value_deltas;
  int retval;
  int curr_big_blind;
  int last_big_blind;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bStud = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-stud"))
      bStud = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  if (!bStud)
    max_streets = 3;
  else
    max_streets = 4;

  ending_balance = -1;

  file_no = 0;
  dbg_file_no = -1;

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

    num_hands = 0;

    line_no = 0;
    bFolded = false;
    bSkipping = false;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (line_no == dbg_line_no)
        dbg = 1;

      if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (Contains(true,
          line,line_len,
          in_chips,IN_CHIPS_LEN,
          &ix)) {

          num_hands++;
          bFolded = false;
          bSkipping = false;

          street = 0;
          num_street_markers = 0;
          ante = 0;
          bring_in = 0;
          spent_this_street = 0;
          spent_this_hand = 0;
          uncalled_bet_amount = 0;
          collected_from_pot = 0;
          collected_from_pot_count = 0;

          line[ix] = 0;

          for (ix--; (ix >= 0) && (line[ix] != '('); ix--)
            ;

          sscanf(&line[ix+1],"%d",&starting_balance);

          continue;
        }
        else if (bSkipping)
          continue;
        else if (Contains(true,
          line,line_len,
          posts_the_ante,POSTS_THE_ANTE_LEN,
          &ix)) {
          ante = get_work_amount(line,line_len);
          spent_this_hand = ante;
          continue;
        }
        else if (bStud && Contains(true,
          line,line_len,
          brings_in_for,BRINGS_IN_FOR_LEN,
          &ix)) {
          bring_in = get_work_amount(line,line_len);
          spent_this_street += bring_in;
          continue;
        }
        else if (Contains(true,
          line,line_len,
          posts,POSTS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;

          continue;
        }
        else if (Contains(true,
          line,line_len,
          collected,COLLECTED_LEN,
          &ix)) {

          for (end_ix = ix + COLLECTED_LEN; end_ix < line_len; end_ix++) {
            if (line[end_ix] == ' ')
              break;
          }

          line[end_ix] = 0;
          sscanf(&line[ix + COLLECTED_LEN],"%d",&work);

          if (!collected_from_pot_count) {
            spent_this_hand += spent_this_street;
            street++;
            spent_this_street = 0;
          }

          collected_from_pot += work;
          collected_from_pot_count++;

          continue;
        }
        else if (!strncmp(line,uncalled_bet,UNCALLED_BET_LEN)) {
          sscanf(&line[UNCALLED_BET_LEN],"%d",&uncalled_bet_amount);
          spent_this_street -= uncalled_bet_amount;

          continue;
        }
        else if (Contains(true,
          line,line_len,
          folds,FOLDS_LEN,
          &ix)) {
          spent_this_hand += spent_this_street;

          bFolded = true;

          ending_balance = starting_balance - spent_this_hand + collected_from_pot;
          delta = ending_balance - starting_balance;
        }
        else if (Contains(true,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;
        }
        else if (Contains(true,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;
        }
        else if (Contains(true,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street = work;
        }
      }
      else {
        if (!strncmp(line,summary,SUMMARY_LEN)) {
          bSkipping = true;

          if (!bFolded) {
            ending_balance = starting_balance - spent_this_hand + collected_from_pot;
            delta = ending_balance - starting_balance;
          }

          if (bTerse)
            printf("%d\n",delta);
          else
            printf("%d %s %d\n",delta,filename,num_hands);
        }
        else if (!strncmp(line,street_marker,STREET_MARKER_LEN)) {
          num_street_markers++;

          if (num_street_markers > 1) {
            if (street <= max_streets)
              spent_this_hand += spent_this_street;

            street++;
            spent_this_street = 0;
          }
        }
      }
    }

    fclose(fptr);
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

    if ((chara == 0xef) || (chara == 0xbb) || (chara == 0xbf))
      continue;

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

int get_work_amount(char *line,int line_len)
{
  int ix;
  int chara;
  int work_amount;

  work_amount = 0;

  for (ix = line_len - 1; (ix >= 0); ix--) {
    chara = line[ix];

    if ((chara >= '0') && (chara <= '9'))
      break;
  }

  if (ix + 1 != line_len);
    line[ix + 1] = 0;

  for ( ; (ix >= 0); ix--) {
    chara = line[ix];

    if ((chara < '0') || (chara > '9'))
      break;
  }

  sscanf(&line[ix+1],"%d",&work_amount);

  return work_amount;
}

static int get_big_blind(
  char *line,
  int line_len,
  int *big_blind_ptr
)
{
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == ')')
      break;
  }

  if (n == line_len)
    return 1;

  for (n--; (n >= 0); n--) {
    if (line[n] == '/')
      break;
  }

  if (n < 0)
    return 2;

  n++;

  sscanf(&line[n],"%d",big_blind_ptr);

  return 0;
}
