#include <stdio.h>
#include <string.h>

#define MAX_SIT_AND_GOS 26

#define MAX_FILENAME_LEN 1024
static char outer_filename[MAX_FILENAME_LEN];
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 4096
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: gen_tournament_summary (-delta) player_name tournament_filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char unexpected_eof[] = "unexpected eof in %s\n";
static char finished[] = "finished the tournament in ";
#define FINISHED_LEN (sizeof (finished) - 1)
static char place_and_received[] = " place and received ";
#define PLACE_AND_RECEIVED_LEN (sizeof (place_and_received) - 1)
static char wins[] = "wins the tournament and receives ";
#define WINS_LEN (sizeof (wins) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static int get_num_hands(char *line,int line_len,int *num_hands);
static int get_buy_in_and_entry_fee(char *line,int line_len,int *buy_in,int *entry_fee);
static int get_num_players(char *line,int line_len,int *num_players);
static int get_place_and_winnings(char *player_name,int player_name_len,FILE *fptr,int *place,int *winnings);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDelta;
  char letter;
  int n;
  FILE *fptr;
  int filename_len;
  int line_len;
  int retval;
  int buy_in;
  int entry_fee;
  int num_players;
  int num_hands;
  int place;
  int winnings;
  int delta;
  int total_delta;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDelta = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-delta"))
      bDelta = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  letter = 'a';

  if (bDelta)
    total_delta = 0;

  printf("buy_in entry players hands place winnings\n\n");

  for (n = 0; n < MAX_SIT_AND_GOS; n++) {
    sprintf(outer_filename,"%c/%s",letter,argv[curr_arg+1]);

    if ((fptr = fopen(outer_filename,"r")) == NULL)
      break;

    GetLine(fptr,line,&line_len,MAX_FILENAME_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,outer_filename);
      return 3;
    }

    fclose(fptr);

    retval = get_num_hands(line,line_len,&num_hands);

    if (retval) {
      printf("get_num_hands failed: %d\n",retval);
      return 4;
    }

    sprintf(filename,"%c/%s",letter,line);

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      return 5;
    }

    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,filename);
      return 6;
    }

    retval = get_buy_in_and_entry_fee(line,line_len,&buy_in,&entry_fee);

    if (retval) {
      printf("get_buy_in_and_entry_fee() failed: %d\n",retval);
      return 7;
    }

    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,filename);
      return 8;
    }

    retval = get_num_players(line,line_len,&num_players);

    if (retval) {
      printf("get_num_players() failed: %d\n",retval);
      return 9;
    }

    if (bDelta)
      delta = (buy_in + entry_fee) * -1;

    retval = get_place_and_winnings(argv[curr_arg],strlen(argv[curr_arg]),fptr,&place,&winnings);

    if (retval) {
      printf("get_place_and_winnings() failed: %d\n",retval);
      return 10;
    }

    fclose(fptr);

    if (!bDelta) {
      printf("%6d %5d %7d %5d %5d %8d\n",
        buy_in,entry_fee,num_players,num_hands,place,winnings);
    }
    else {
      delta += winnings;
      printf("%6d %5d %7d %5d %5d %8d %8d\n",
        buy_in,entry_fee,num_players,num_hands,place,winnings,delta);
      total_delta += delta;
    }

    letter++;
  }

  if (bDelta)
    printf("\n                                          %8d\n",total_delta);

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

static int get_num_players(char *line,int line_len,int *num_players)
{
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == '-')
      break;
  }

  if (n == line_len)
    return 1;

  line[n] = 0;

  sscanf(&line[n-1],"%d",num_players);

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

        if (Contains(true,
          line,line_len,
          place_and_received,PLACE_AND_RECEIVED_LEN,
          &ix)) {

          sscanf(&line[ix + PLACE_AND_RECEIVED_LEN],"%d",winnings);
        }

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
