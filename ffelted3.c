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
static char prev_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: ffelted3 (-debug) (-reverse) (-count) (-exact_countn) (-reupped)\n"
"  (-consecutive) (-show_zero) (-one_and_done) (-stud)\n"
"  (-max_felt_distance) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char fmt[] = "%d\t%s\n";

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
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bDebug;
  bool bReverse;
  bool bCount;
  bool bExactCount;
  bool bReupped;
  bool bConsecutive;
  bool bShowZero;
  bool bOneAndDone;
  bool bStud;
  bool bMaxFeltDistance;
  int hit_felt_count;
  int reupped_count;
  int consecutive_hit_felt_count;
  int exact_count;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
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
  int file_no;
  int dbg_file_no;
  int dbg;
  int work;
  double dwork1;
  double dwork2;
  bool bSkipping;
  int hand_number;
  int last_felted_hand_number;
  int felt_distance;
  int max_felt_distance;
  char *date_string;

  if ((argc < 3) || (argc > 13)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bReverse = false;
  bCount = false;
  bExactCount = false;
  bReupped = false;
  bConsecutive = false;
  bShowZero = false;
  bOneAndDone = false;
  bStud = false;
  bMaxFeltDistance = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = true;
    else if (!strcmp(argv[curr_arg],"-count"))
      bCount = true;
    else if (!strncmp(argv[curr_arg],"-exact_count",12)) {
      bExactCount = true;
      sscanf(&argv[curr_arg][12],"%d",&exact_count);
    }
    else if (!strcmp(argv[curr_arg],"-reupped"))
      bReupped = true;
    else if (!strcmp(argv[curr_arg],"-consecutive"))
      bConsecutive = true;
    else if (!strcmp(argv[curr_arg],"-show_zero")) {
      bShowZero = true;
      bCount = true;
    }
    else if (!strcmp(argv[curr_arg],"-one_and_done"))
      bOneAndDone = true;
    else if (!strcmp(argv[curr_arg],"-stud"))
      bStud = true;
    else if (!strcmp(argv[curr_arg],"-max_felt_distance"))
      bMaxFeltDistance = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (bExactCount && bOneAndDone) {
    printf("can't specify boht -exact_countn and -one_and_done\n");
    return 3;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  if (!bStud)
    max_streets = 3;
  else
    max_streets = 4;

  file_no = 0;
  dbg_file_no = -1;

  if (bConsecutive)
    consecutive_hit_felt_count = 0;

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

    retval = get_date_from_path(filename,'\\',3,&date_string);

    if (retval) {
      printf("get_date_from_path() on %s failed: %d\n",filename,retval);
      continue;
    }

    strcpy(prev_filename,date_string);

    line_no = 0;
    hit_felt_count = 0;
    ending_balance = -1;
    reupped_count = 0;

    if (bMaxFeltDistance) {
      hand_number = 0;
      last_felted_hand_number = 0;
      max_felt_distance = 0;
    }

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (line_no == dbg_line_no)
        dbg = 1;

      if (bDebug)
        printf("line %d %s\n",line_no,line);

      if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (Contains(true,
          line,line_len,
          in_chips,IN_CHIPS_LEN,
          &ix)) {

          if (bMaxFeltDistance)
            hand_number++;

          bSkipping = false;

          if (ending_balance == 0)
            reupped_count++;

          street = 0;
          num_street_markers = 0;
          spent_this_street = 0;
          spent_this_hand = 0;
          uncalled_bet_amount = 0;
          collected_from_pot = 0;
          collected_from_pot_count = 0;

          line[ix] = 0;

          for (ix--; (ix >= 0) && (line[ix] != '('); ix--)
            ;

          sscanf(&line[ix+1],"%d",&starting_balance);

          if (bDebug)
            printf("line %d starting_balance = %d\n",line_no,starting_balance);

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
        else if (Contains(true,
          line,line_len,
          posts,POSTS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;

          if (bDebug) {
            printf("line %d street %d POSTS work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }

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

          if (bDebug) {
            printf("line %d street %d COLLECTED work = %d, collected_from_pot = %d\n",
              line_no,street,work,collected_from_pot);
          }

          continue;
        }
        else if (!strncmp(line,uncalled_bet,UNCALLED_BET_LEN)) {
          sscanf(&line[UNCALLED_BET_LEN],"%d",&uncalled_bet_amount);
          spent_this_street -= uncalled_bet_amount;

          if (bDebug) {
            printf("line %d street %d UNCALLED uncalled_bet_amount = %d, spent_this_street = %d\n",
              line_no,street,uncalled_bet_amount,spent_this_street);
          }

          continue;
        }
        else if (Contains(true,
          line,line_len,
          folds,FOLDS_LEN,
          &ix)) {
          spent_this_hand += spent_this_street;

          if (bDebug) {
            printf("line %d street %d FOLDS spent_this_street = %d, spent_this_hand = %d\n",
              line_no,street,spent_this_street,spent_this_hand);
          }

          bSkipping = true;

          ending_balance = starting_balance - spent_this_hand + collected_from_pot;

          continue;
        }
        else if (Contains(true,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;

          if (bDebug) {
            printf("line %d street %d BETS work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }
        }
        else if (Contains(true,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;

          if (bDebug) {
            printf("line %d street %d CALLS work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }
        }
        else if (Contains(true,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street = work;

          if (bDebug) {
            printf("line %d street %d RAISES work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }
        }
        else if (bStud && Contains(true,
          line,line_len,
          brings_in_for,BRINGS_IN_FOR_LEN,
          &ix)) {
          bring_in = get_work_amount(line,line_len);
          spent_this_street += bring_in;
          continue;
        }
      }
      else if (bSkipping)
        continue;
      else {
        if (!strncmp(line,summary,SUMMARY_LEN)) {
          if (bDebug)
            printf("line %d SUMMARY line detected; skipping\n",line_no);

          bSkipping = true;

          ending_balance = starting_balance - spent_this_hand + collected_from_pot;

          if (!ending_balance) {
            hit_felt_count++;

            if (bMaxFeltDistance) {
              felt_distance = hand_number - last_felted_hand_number;

              if (felt_distance > max_felt_distance)
                max_felt_distance = felt_distance;

              last_felted_hand_number = hand_number;
            }
          }

          continue;
        }

        if (!strncmp(line,street_marker,STREET_MARKER_LEN)) {
          num_street_markers++;

          if (num_street_markers > 1) {
            if (street <= num_street_markers)
              spent_this_hand += spent_this_street;

            street++;
            spent_this_street = 0;
          }
        }
      }
    }

    if (bOneAndDone) {
      if ((hit_felt_count == 1) && (ending_balance == 0))
        printf("%s\n",filename);
    }
    else if (!bReverse) {
      if (bReupped && (reupped_count == 0))
        ;
      else if (bConsecutive) {
        consecutive_hit_felt_count += hit_felt_count;

        if (ending_balance && (consecutive_hit_felt_count > 0)) {
          printf(fmt,consecutive_hit_felt_count,date_string);
          consecutive_hit_felt_count = 0;
        }
      }
      else if ((bShowZero || hit_felt_count) && (!bExactCount || (hit_felt_count == exact_count))) {
        if (!bCount) {
          if (!bMaxFeltDistance)
            printf("%s\n",filename);
          else
            printf(fmt,max_felt_distance,date_string);
        }
        else
          printf(fmt,hit_felt_count,date_string);
      }
    }
    else {
      if (!hit_felt_count)
        printf("%s\n",filename);
    }

    fclose(fptr);
  }

  if (bConsecutive && (consecutive_hit_felt_count > 0))
    printf(fmt,consecutive_hit_felt_count,prev_filename);

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

static char sql_date_string[11];

static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr)
{
  int n;
  int len;
  int slash_count;

  len = strlen(path);
  slash_count = 0;

  for (n = len - 1; (n >= 0); n--) {
    if (path[n] == slash_char) {
      slash_count++;

      if (slash_count == num_slashes)
        break;
    }
  }

  if (slash_count != num_slashes)
    return 1;

  if (path[n+5] != slash_char)
    return 2;

  strncpy(sql_date_string,&path[n+1],4);
  sql_date_string[4] = '-';
  strncpy(&sql_date_string[5],&path[n+6],2);
  sql_date_string[7] = '-';
  strncpy(&sql_date_string[8],&path[n+8],2);
  sql_date_string[10] = 0;

  *date_string_ptr = sql_date_string;

  return 0;
}
