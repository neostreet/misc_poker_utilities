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
"usage: fshort_stack (-verbose) (-count) (-only_countcount) player_name filename\n";
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
  bool bCount;
  bool bOnlyCount;
  int only_count;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filenamelen;
  int hand;
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;
  int table_count;
  int min_table_chips;
  int min_table_chips_count;
  int player_chips;
  int work;
  int short_stack_count;

  if ((argc < 3) && (argc > 6)) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  bVerbose = false;
  bCount = false;
  bOnlyCount = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-count"))
      bCount = true;
    else if (!strncmp(argv[curr_arg],"-only_count",11)) {
      bOnlyCount = true;
      sscanf(&argv[curr_arg][11],"%d",&only_count);
    }
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

  hand = 0;

  if (bCount)
    short_stack_count = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filenamelen,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    hand++;

    line_no = 0;
    player_chips = 0;
    table_count = 0;
    min_table_chips = -1;
    min_table_chips_count = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
        line,line_len,
        in_chips,IN_CHIPS_LEN,
        &ix)) {

        table_count++;
        line[ix] = 0;

        for (ix--; (ix >= 0) && (line[ix] != '('); ix--)
          ;

        sscanf(&line[ix+1],"%d",&work);

        if ((min_table_chips == -1) || (work <= min_table_chips)) {
          min_table_chips = work;
          min_table_chips_count++;
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

    if ((player_chips == min_table_chips) && (min_table_chips_count == 1)) {
      if (!bOnlyCount || (table_count == only_count)) {
        if (bCount)
          short_stack_count++;
        else if (!bVerbose)
          printf("%d %d (hand %d)\n",min_table_chips,table_count,hand);
        else {
          printf("%d %d %s/%s (hand %d)\n",min_table_chips,table_count,
            save_dir,filename,hand);
        }
      }
    }
  }

  if (bCount) {
    if (short_stack_count) {
      if (!bVerbose)
        printf("%d\n",short_stack_count);
      else
        printf("%d %s\n",short_stack_count,save_dir);
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
