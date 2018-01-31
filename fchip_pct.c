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
"usage: fchip_pct (-verbose) (-max) (-min) (-last) (-total_chipschips)\n"
"  (-first_handhand) player_name filename\n";
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
  int max;
  int min;
  int last;
  bool bHaveTotalChips;
  int total_chips;
  int num_players;
  int first_hand;
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
  int save_player_chips;
  int work;
  double dwork;
  double save_dwork;

  if ((argc < 3) && (argc > 9)) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  bVerbose = false;
  max = 0;
  min = 0;
  last= 0;
  bHaveTotalChips = false;
  total_chips = -1;
  first_hand = 1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-max"))
      max = 1;
    else if (!strcmp(argv[curr_arg],"-min"))
      min = 1;
    else if (!strcmp(argv[curr_arg],"-last"))
      last = 1;
    else if (!strncmp(argv[curr_arg],"-total_chips",12)) {
      bHaveTotalChips = true;
      sscanf(&argv[curr_arg][12],"%d",&total_chips);
    }
    else if (!strncmp(argv[curr_arg],"-first_hand",11))
      sscanf(&argv[curr_arg][11],"%d",&first_hand);
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (max + min + last > 1) {
    printf("specify at most one of -max, -min, and -last\n");
    return 3;
  }

  if (max || min)
    save_dwork = (double)0;

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

    if (file_no < first_hand)
      continue;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    player_chips = 0;

    if (!bHaveTotalChips) {
      total_chips = 0;
      num_players = 0;
    }

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

        if (!bHaveTotalChips) {
          total_chips += work;
          num_players++;
        }

        if (Contains(true,
          line,line_len,
          argv[player_name_ix],player_name_len,
          &ix)) {
          player_chips = work;
        }
      }
    }

    fclose(fptr);

    dwork = (double)player_chips / (double)total_chips;

    if (max) {
      if (dwork > save_dwork) {
        save_dwork = dwork;
        strcpy(save_filename,filename);

        if (bVerbose)
          save_player_chips = player_chips;
      }
    }
    else if (min) {
      if ((save_dwork == (double)0) || (dwork < save_dwork)) {
        save_dwork = dwork;
        strcpy(save_filename,filename);

        if (bVerbose)
          save_player_chips = player_chips;
      }
    }
    else if (last) {
      save_dwork = dwork;
      strcpy(save_filename,filename);

      if (bVerbose)
        save_player_chips = player_chips;
    }
    else {
      if (!bVerbose)
        printf("%7.4lf %s/%s\n",dwork,save_dir,filename);
      else {
        printf("%7.4lf (%d %10d %10d) %s/%s\n",dwork,
          num_players,player_chips,total_chips,save_dir,filename);
      }
    }
  }

  if (max || min || last) {
    if (!bVerbose)
      printf("%7.4lf %s/%s\n",save_dwork,save_dir,save_filename);
    else {
      printf("%7.4lf (%10d %10d) %s/%s\n",save_dwork,
        save_player_chips,total_chips,save_dir,save_filename);
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
