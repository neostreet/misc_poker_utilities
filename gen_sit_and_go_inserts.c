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

#define MAX_SIT_AND_GOS 26

#define MAX_FILENAME_LEN 1024
static char outer_filename[MAX_FILENAME_LEN];
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 4096
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: sit_and_go_inserts player_name initial_stake big_blind_amount\n";
static char couldnt_open[] = "couldn't open %s\n";
static char unexpected_eof[] = "unexpected eof in %s\n";
static char finished[] = "finished the tournament in ";
#define FINISHED_LEN (sizeof (finished) - 1)
static char finished_in_second_place[] = "finished the tournament in 2nd place and received ";
#define FINISHED_IN_SECOND_PLACE_LEN (sizeof (finished_in_second_place) - 1)
static char wins[] = "wins the tournament and receives ";
#define WINS_LEN (sizeof (wins) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static int get_num_hands(char *line,int line_len,int *num_hands);
static int get_buy_in_and_entry_fee(char *line,int line_len,int *buy_in,int *entry_fee);
static int get_place_and_winnings(char *player_name,int player_name_len,FILE *fptr,int *place,int *winnings);

int main(int argc,char **argv)
{
  char letter;
  int n;
  FILE *fptr;
  int filename_len;
  int line_len;
  int retval;
  char *date_string;
  int buy_in;
  int entry_fee;
  int num_players;
  int poker_flavor;
  int num_hands;
  int place;
  int winnings;

  if (argc != 4) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  retval = get_date_from_path(save_dir,'/',2,&date_string);

  if (retval) {
    printf("get_date_from_path() failed: %d\n",retval);
    return 5;
  }

  letter = 'a';
  num_players = 6;
  poker_flavor = 3;

  for (n = 0; n < MAX_SIT_AND_GOS; n++) {
    sprintf(outer_filename,"%c/hands.ls0",letter);

    if ((fptr = fopen(outer_filename,"r")) == NULL)
      break;

    GetLine(fptr,line,&line_len,MAX_FILENAME_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,outer_filename);
      return 2;
    }

    fclose(fptr);

    retval = get_num_hands(line,line_len,&num_hands);

    if (retval) {
      printf("get_num_hands failed: %d\n",retval);
      return 3;
    }

    sprintf(filename,"%c/%s",letter,line);

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      return 4;
    }

    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,filename);
      return 5;
    }

    retval = get_buy_in_and_entry_fee(line,line_len,&buy_in,&entry_fee);

    if (retval) {
      printf("get_buy_in_and_entry_fee failed: %d\n",retval);
      return 6;
    }

    retval = get_place_and_winnings(argv[1],strlen(argv[1]),fptr,&place,&winnings);

    fclose(fptr);

    printf("insert into poker_sessions (\n");
    printf("  sit_and_go,poker_session_date,buy_in,entry_fee,initial_stake,"
      "big_blind_amount,num_players,poker_flavor,num_hands,place,winnings\n");
    printf(")\n");
    printf("values (1,'%s',%d,%d,%s,%s,%d,%d,%d,%d,%d);\n",
      date_string,buy_in,entry_fee,argv[2],argv[3],num_players,poker_flavor,
      num_hands,place,winnings);

    letter++;
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

static int get_buy_in_and_entry_fee(char *line,int line_len,int *buy_in,int *entry_fee)
{
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == '+')
      break;
  }

  if (n == line_len)
    return 1;

  line[n] = 0;

  sscanf(&line[n+1],"%d",entry_fee);

  for (n--; (n >= 0); n--) {
    if (line[n] == ' ')
      break;
  }

  if (n < 0)
    return 2;

  sscanf(&line[n+1],"%d",buy_in);

  return 0;
}

static int get_num_hands(char *line,int line_len,int *num_hands)
{
  int m;
  int n;
  char buf[20];

  for (n = 0; n < line_len; n++) {
    if ((line[n] >= '0') && (line[n] <= '9'))
      break;
  }

  if (n == line_len)
    return 1;

  for (m = n; m < line_len; m++) {
    if (line[m] == '.')
      break;

    buf[m - n] = line[m];
  }

  if (m == line_len)
    return 2;

  buf[m - n] = 0;

  sscanf(buf,"%d",num_hands);

  return 0;
}

static int get_place_and_winnings(char *player_name,int player_name_len,FILE *fptr,int *place,int *winnings)
{
  int ix;
  int line_len;

  *place = 0;
  *winnings = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_FILENAME_LEN);

    if (feof(fptr))
      break;

    if (Contains(true,
      line,line_len,
      player_name,player_name_len,
      &ix)) {

      if (Contains(true,
        line,line_len,
        finished,FINISHED_LEN,
        &ix)) {

        sscanf(&line[ix + FINISHED_LEN],"%d",place);

        if (*place == 2)
          sscanf(&line[ix + FINISHED_IN_SECOND_PLACE_LEN],"%d",winnings);

        break;
      }

      if (Contains(true,
        line,line_len,
        wins,WINS_LEN,
        &ix)) {

        *place = 1;
        sscanf(&line[ix + WINS_LEN],"%d",winnings);

        break;
      }
    }
  }

  return 0;
}