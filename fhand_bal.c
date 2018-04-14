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
static char min_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fhand_bal (-debug) (-consistency) (-delta) (-starting_balance) (-terse)\n"
"  (-double_zero) (-no_balances) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char pokerstars[] = "PokerStars";
#define POKERSTARS_LEN (sizeof (pokerstars) - 1)
static char stud[] = "7 Card Stud";
#define STUD_LEN (sizeof (stud) - 1)
static char razz[] = "Razz";
#define RAZZ_LEN (sizeof (razz) - 1)
static char eight_game[] = "8-Game";
#define EIGHT_GAME_LEN (sizeof (eight_game) - 1)
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
static char uncalled_bet[] = "Uncalled bet (";
#define UNCALLED_BET_LEN (sizeof (uncalled_bet) - 1)
static char collected[] = " collected ";
#define COLLECTED_LEN (sizeof (collected) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
int get_work_amount(char *line,int line_len);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bConsistency;
  bool bDelta;
  bool bStartingBalance;
  bool bTerse;
  bool bDoubleZero;
  bool bNoBalances;
  bool bStud;
  bool bRazz;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
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
  int wagered_amount;
  int uncalled_bet_amount;
  int collected_from_pot;
  int collected_from_pot_count;
  int ending_balance;
  int delta;
  int max_ending_balance;
  int min_ending_balance;
  int max_delta;
  int min_delta;
  int max_starting_balance;
  int min_starting_balance;
  int file_no;
  int dbg_file_no;
  int dbg;
  int work;
  double dwork;
  int prev_ending_balance;

  if ((argc < 3) || (argc > 10)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bConsistency = false;
  bDelta = false;
  bStartingBalance = false;
  bTerse = false;
  bDoubleZero = false;
  bNoBalances = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-consistency"))
      bConsistency = true;
    else if (!strcmp(argv[curr_arg],"-delta"))
      bDelta = true;
    else if (!strcmp(argv[curr_arg],"-starting_balance"))
      bStartingBalance = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-double_zero"))
      bDoubleZero = true;
    else if (!strcmp(argv[curr_arg],"-no_balances"))
      bNoBalances = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (bDelta && bStartingBalance) {
    printf("can't specify both -delta and -starting_balance\n");
    return 3;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  ending_balance = -1;
  prev_ending_balance = -1;

  if (bDelta) {
    max_delta = -1;
    min_delta = 1;
  }
  else if (bStartingBalance) {
    max_starting_balance = -1;
    min_starting_balance = 1;
  }
  else {
    max_ending_balance = -1;
    min_ending_balance = 1;
  }

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

    line_no = 0;
    street = 0;
    num_street_markers = 0;
    ante = 0;
    bring_in = 0;
    spent_this_street = 0;
    spent_this_hand = 0;
    uncalled_bet_amount = 0;
    collected_from_pot = 0;
    collected_from_pot_count = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
        line,line_len,
        pokerstars,POKERSTARS_LEN,
        &ix)) {

        bStud = false;
        bRazz = false;

        if (Contains(true,
          line,line_len,
          stud,STUD_LEN,
          &ix)) {

          bStud = true;
        }
        else if (Contains(true,
          line,line_len,
          razz,RAZZ_LEN,
          &ix)) {

          bRazz = true;
        }

        if (bStud || bRazz)
          max_streets = 4;
        else
          max_streets = 3;
      }
      else if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (Contains(true,
          line,line_len,
          in_chips,IN_CHIPS_LEN,
          &ix)) {

          line[ix] = 0;

          for (ix--; (ix >= 0) && (line[ix] != '('); ix--)
            ;

          sscanf(&line[ix+1],"%d",&starting_balance);

          if (bConsistency) {
            if ((ending_balance != -1) &&
                (ending_balance != starting_balance))
              printf("discrepancy: %d %s: starting balance of %d != "
                "last ending balance of %d\n",
                starting_balance - ending_balance,filename,
                starting_balance,ending_balance);
          }

          continue;
        }
        else if (Contains(true,
          line,line_len,
          posts_the_ante,POSTS_THE_ANTE_LEN,
          &ix)) {
          ante = get_work_amount(line,line_len);
          spent_this_hand = ante;
          continue;
        }
        else if ((bStud || bRazz) && Contains(true,
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
          spent_this_street += get_work_amount(line,line_len);
          continue;
        }
        else if (!strncmp(line,dealt_to,DEALT_TO_LEN))
          continue;
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
          break;
        }
        else if (Contains(true,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {
          spent_this_street += get_work_amount(line,line_len);
        }
        else if (Contains(true,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {
          spent_this_street += get_work_amount(line,line_len);
        }
        else if (Contains(true,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {
          spent_this_street = get_work_amount(line,line_len);
        }
      }
      else {
        if (!strncmp(line,summary,SUMMARY_LEN))
          break;

        if (!strncmp(line,street_marker,STREET_MARKER_LEN)) {
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

    ending_balance = starting_balance - spent_this_hand + collected_from_pot;
    delta = ending_balance - starting_balance;

    if (bDelta) {
      if ((max_delta == -1) || (delta > max_delta)) {
        max_delta = delta;
        strcpy(max_filename,filename);
      }

      if ((min_delta == 1) || (delta < min_delta)) {
        min_delta = delta;
        strcpy(min_filename,filename);
      }
    }
    else if (bStartingBalance) {
      if ((max_starting_balance == -1) || (starting_balance > max_starting_balance)) {
        max_starting_balance = starting_balance;
        strcpy(max_filename,filename);
      }

      if ((min_starting_balance == 1) || (starting_balance < min_starting_balance)) {
        min_starting_balance = starting_balance;
        strcpy(min_filename,filename);
      }
    }
    else {
      if ((max_ending_balance == -1) || (ending_balance > max_ending_balance)) {
        max_ending_balance = ending_balance;
        strcpy(max_filename,filename);
      }

      if ((min_ending_balance == 1) || (ending_balance < min_ending_balance)) {
        min_ending_balance = ending_balance;
        strcpy(min_filename,filename);
      }
    }

    if (bDoubleZero) {
      if ((prev_ending_balance == 0) && (ending_balance == 0))
        ;
      else {
        prev_ending_balance = ending_balance;
        continue;
      }
    }

    prev_ending_balance = ending_balance;

    if (bNoBalances)
      continue;

    if (!bDebug) {
      if (bTerse) {
        if (bDelta)
          printf("%d\n",delta);
        else {
          printf("%10d %10d %10d %s/%s\n",
            starting_balance,delta,ending_balance,save_dir,filename);
        }
      }
      else if (bDelta)
        printf("%10d %s/%s\n",delta,save_dir,filename);
      else if (bStartingBalance)
        printf("%10d %s/%s\n",starting_balance,save_dir,filename);
      else
        printf("%10d %s/%s\n",ending_balance,save_dir,filename);
    }
    else {
      wagered_amount = spent_this_hand + uncalled_bet_amount;

      printf("%s/%s\n",save_dir,filename);
      printf("%10d starting_balance\n",starting_balance);

      if (ante)
        printf("%10d ante\n",ante);

      if (bring_in)
        printf("%10d bring_in\n",bring_in);

      printf("%10d wagered_amount\n",wagered_amount);
      printf("%10d uncalled_bet_amount\n",uncalled_bet_amount);
      printf("%10d spent_this_hand\n",spent_this_hand);
      printf("%10d collected_from_pot\n",collected_from_pot);
      printf("%10d ending_balance\n",ending_balance);
      printf("%10d delta\n",delta);
      printf("%10d num_street_markers\n",num_street_markers);
      printf("%10d streets\n",street);

      if (collected_from_pot && (delta > 0)) {
        dwork = (double)delta / (double)collected_from_pot;
        printf("%10.2lf opm_percentage\n",dwork);
        dwork = (double)ending_balance / (double)starting_balance;
        printf("%10.2lf starting_balance_multiplier\n",dwork);
      }
    }
  }

  fclose(fptr0);

  if (bDoubleZero || bTerse)
    return 0;

  if (bNoBalances)
    return 0;

  if (bDelta) {
    if (max_delta != -1)
      printf("\n%10d %s\n",max_delta,max_filename);

    if (min_delta != 1)
      printf("%10d %s\n",min_delta,min_filename);
  }
  else if (bStartingBalance) {
    if (max_starting_balance != -1)
      printf("\n%10d %s\n",max_starting_balance,max_filename);

    if (min_starting_balance != 1)
      printf("%10d %s\n",min_starting_balance,min_filename);
  }
  else {
    if (max_ending_balance != -1)
      printf("\n%10d %s\n",max_ending_balance,max_filename);

    if (min_ending_balance != 1)
      printf("%10d %s\n",min_ending_balance,min_filename);
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
