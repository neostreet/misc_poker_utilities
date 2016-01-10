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
"usage: min_chip_pct (-verbose) (-heads_up) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  bool bHeadsUp;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  int ix;
  int chips;
  int total_chips_in_play;
  int my_chips;
  double my_percent;
  int min_chip_pct_num;
  int min_chip_pct_denom;
  double min_chip_pct;
  int min_chip_hand;
  int num_files;
  int dbg_num_files;
  int dbg;
  FILE *fptr;
  int line_len;
  int table_count;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bHeadsUp = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-heads_up"))
      bHeadsUp = true;
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

  num_files = 0;
  min_chip_pct = (double)100;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    num_files++;

    if (num_files == dbg_num_files)
      dbg = 1;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (!strncmp(line,"Table '",7)) {
        table_count = 0;
        total_chips_in_play = 0;

        for ( ; ; ) {
          GetLine(fptr,line,&line_len,MAX_LINE_LEN);

          if (feof(fptr))
            break;

          if (!strncmp(line,street_marker,STREET_MARKER_LEN))
            break;

          if (!strncmp(line,"Seat ",5)) {
            table_count++;

            if (Contains(true,
              line,line_len,
              in_chips,IN_CHIPS_LEN,
              &ix)) {

              line[ix] = 0;

              for (ix--; (ix >= 0) && (line[ix] != '('); ix--)
                ;

              sscanf(&line[ix+1],"%d",&chips);
            }

            total_chips_in_play += chips;

            if (Contains(true,
              line,line_len,
              argv[player_name_ix],player_name_len,
              &ix)) {

              my_chips = chips;
            }
          }
        }

        if (bHeadsUp && (table_count != 2))
          break;

        my_percent = (double)my_chips / (double)total_chips_in_play * (double)100;

        if (my_percent < min_chip_pct) {
          min_chip_pct = my_percent;

          if (bVerbose) {
            min_chip_pct_num = my_chips;
            min_chip_pct_denom = total_chips_in_play;
          }

          min_chip_hand = num_files;
        }

        break;
      }
    }

    fclose(fptr);
  }

  fclose(fptr0);

  if (min_chip_pct != (double)100) {
    if (!bVerbose)
      printf("%lf hand %d\n",min_chip_pct,min_chip_hand);
    else {
      printf("%lf (%d %d) hand %d %s\n",min_chip_pct,
        min_chip_pct_num,min_chip_pct_denom,
        min_chip_hand,save_dir);
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
