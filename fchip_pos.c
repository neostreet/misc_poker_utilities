#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

struct chip_counts {
  int count;
  int me;
};

#define MAX_TABLE_PLAYERS 9
static struct chip_counts table_chip_counts[MAX_TABLE_PLAYERS+1];

static char save_dir[_MAX_PATH];

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fchip_pos (-verbose) (-first_handhand) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
int compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
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
  int work;
  double dwork;
  double save_dwork;
  int ixs[MAX_TABLE_PLAYERS];

  if ((argc < 3) && (argc > 5)) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  bVerbose = false;
  first_hand = 1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strncmp(argv[curr_arg],"-first_hand",11))
      sscanf(&argv[curr_arg][11],"%d",&first_hand);
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
    num_players = 0;

    for (n = 0; n < MAX_TABLE_PLAYERS+1; n++) {
      table_chip_counts[n].count = 0;
      table_chip_counts[n].me = 0;
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
        table_chip_counts[num_players].count = work;
        table_chip_counts[MAX_TABLE_PLAYERS].count += work;

        if (Contains(true,
          line,line_len,
          argv[player_name_ix],player_name_len,
          &ix)) {
          table_chip_counts[num_players].me = 1;
        }

        num_players++;
      }
      else if (num_players)
        break;
    }

    fclose(fptr);

    for (n = 0; n < num_players; n++)
      ixs[n] = n;

    qsort(ixs,num_players,sizeof (int),compare);

    for (n = 0; n < num_players; n++) {
      if (table_chip_counts[ixs[n]].me) {
        if (!bVerbose)
          printf("%d\n",n+1);
        else {
          printf("%d (%d %d) %s/%s\n",n+1,
            table_chip_counts[ixs[n]].count,
            table_chip_counts[MAX_TABLE_PLAYERS].count,
            save_dir,filename);
        }

        break;
      }
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

int compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;
  int int1;
  int int2;

  ix1 = *(int *)elem1;
  ix2 = *(int *)elem2;

  int1 = table_chip_counts[ix1].count;
  int2 = table_chip_counts[ix2].count;

  return int2 - int1;
}
