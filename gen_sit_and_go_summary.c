#include <stdio.h>
#include <string.h>

#define MAX_SIT_AND_GOS 26

#define MAX_FILENAME_LEN 1024
static char outer_filename[MAX_FILENAME_LEN];
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 4096
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: gen_sit_and_go_summary (-delta) player_name poker_style poker_flavor\n";
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

static char *poker_styles[] = {
  "Cash ",
  "SNG  ",
  "MTT  ",
  "Sp&Go",
  "KO   "
};
#define NUM_POKER_STYLES (sizeof poker_styles / sizeof (char *))

static char *poker_flavors[] = {
  "PLHE  ",
  "PLO   ",
  "7Stud ",
  "NLHE  ",
  "L5draw",
  "NL27Lo",
  "PL27Lo",
  "LHE   ",
  "L27Lo ",
  "8-game",
  "HORSE "
};
#define NUM_POKER_FLAVORS (sizeof poker_flavors / sizeof (char *))

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDelta;
  int poker_style;
  int poker_flavor;
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

  if ((argc < 4) || (argc > 5)) {
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

  if (argc - curr_arg != 3) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg+1],"%d",&poker_style);

  if ((poker_style < 0) || (poker_style >= NUM_POKER_STYLES)) {
    printf("invalid poker_style\n");
    return 3;
  }

  sscanf(argv[curr_arg+2],"%d",&poker_flavor);

  if ((poker_flavor < 0) || (poker_flavor >= NUM_POKER_FLAVORS)) {
    printf("invalid poker_flavor\n");
    return 4;
  }

  letter = 'a';

  if (bDelta)
    total_delta = 0;

  if (((poker_style == 1) && (poker_flavor == 3)) ||
      ((poker_style == 4) && (poker_flavor == 3)))
    printf("style flavor buy_in entry players hands entries paid place winnings\n\n");
  else
    printf("style flavor buy_in entry players hands place winnings\n\n");

  for (n = 0; n < MAX_SIT_AND_GOS; n++) {
    sprintf(outer_filename,"%c/sng_hands.ls0",letter);

    if ((fptr = fopen(outer_filename,"r")) == NULL)
      break;

    GetLine(fptr,line,&line_len,MAX_FILENAME_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,outer_filename);
      return 5;
    }

    fclose(fptr);

    retval = get_num_hands(line,line_len,&num_hands);

    if (retval) {
      printf("get_num_hands failed: %d\n",retval);
      return 6;
    }

    sprintf(filename,"%c/%s",letter,line);

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      return 7;
    }

    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,filename);
      return 8;
    }

    retval = get_buy_in_and_entry_fee(line,line_len,&buy_in,&entry_fee);

    if (retval) {
      printf("get_buy_in_and_entry_fee() failed: %d\n",retval);
      return 9;
    }

    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr)) {
      printf(unexpected_eof,filename);
      return 10;
    }

    retval = get_num_players(line,line_len,&num_players);

    if (retval) {
      printf("get_num_players() failed: %d\n",retval);
      return 11;
    }

    if (bDelta)
      delta = (buy_in + entry_fee) * -1;

    retval = get_place_and_winnings(argv[curr_arg],strlen(argv[curr_arg]),fptr,&place,&winnings);

    if (retval) {
      printf("get_place_and_winnings() failed: %d\n",retval);
      return 12;
    }

    fclose(fptr);

    if (!bDelta) {
      if ((poker_style == 1) && (poker_flavor == 3)) {
        printf("%5s %6s %6d %5d %7d %5d %7d %4d %5d %8d\n",
          poker_styles[poker_style],poker_flavors[poker_flavor],
          buy_in,entry_fee,num_players,num_hands,6,2,place,winnings);
      }
      else if ((poker_style == 4) && (poker_flavor == 3)) {
        printf("%5s %6s %6d %5d %7d %5d %7d %4d %5d %8d\n",
          poker_styles[poker_style],poker_flavors[poker_flavor],
          buy_in,entry_fee,num_players,num_hands,9,3,place,winnings);
      }
      else {
        printf("%5s %6s %6d %5d %7d %5d %5d %8d\n",
          poker_styles[poker_style],poker_flavors[poker_flavor],
          buy_in,entry_fee,num_players,num_hands,place,winnings);
      }
    }
    else {
      delta += winnings;

      if ((poker_style == 1) && (poker_flavor == 3)) {
        printf("%5s %6s %6d %5d %7d %5d %7d %4d %5d %8d %8d\n",
          poker_styles[poker_style],poker_flavors[poker_flavor],
          buy_in,entry_fee,num_players,num_hands,6,2,place,winnings,delta);
      }
      else if ((poker_style == 4) && (poker_flavor == 3)) {
        printf("%5s %6s %6d %5d %7d %5d %7d %4d %5d %8d %8d\n",
          poker_styles[poker_style],poker_flavors[poker_flavor],
          buy_in,entry_fee,num_players,num_hands,9,3,place,winnings,delta);
      }
      else {
        printf("%5s %6s %6d %5d %7d %5d %5d %8d %8d\n",
          poker_styles[poker_style],poker_flavors[poker_flavor],
          buy_in,entry_fee,num_players,num_hands,place,winnings,delta);
      }

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
  int work;

  *buy_in = 0;

  for (n = line_len - 1; (n >= 0); n--) {
    if (line[n] == '+')
      break;
  }

  if (n < 0)
    return 1;

  line[n] = 0;

  sscanf(&line[n+1],"%d",entry_fee);

  for (n--; (n >= 0); n--) {
    if (line[n] == '+') {
      line[n] = 0;
      sscanf(&line[n+1],"%d",&work);
      *buy_in += work;

      n--;

      if (n < 0)
        break;
    }

    if (line[n] == ' ')
      break;
  }

  if (n < 0)
    return 2;

  sscanf(&line[n+1],"%d",&work);
  *buy_in += work;

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
