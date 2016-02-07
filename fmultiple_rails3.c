#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif
using namespace std;

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fmultiple_rails3 (-terse) (-verbose) (-debug)\n"
"  (-player_namename) (-player_hit_rail) (-verbose_style2) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)

static char finished[] = "finished the tournament";
#define FINISHED_LEN (sizeof (finished) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static char *style2(char *filename);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int q;
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  bool bVerboseStyle2;
  bool bDebug;
  bool bHavePlayerName;
  char *player_name;
  int player_name_len;
  bool bHavePlayerHitRail;
  bool bPlayerHitRail;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int dbg_line_no;
  int file_no;
  int dbg_file_no;
  int num_hands;
  int dbg;
  int table_count;
  int finished_count;
  int ix;

  if ((argc < 2) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  bVerboseStyle2 = false;
  bDebug = false;
  bHavePlayerName = false;
  bHavePlayerHitRail = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strncmp(argv[curr_arg],"-player_name",12)) {
      player_name = &argv[curr_arg][12];
      player_name_len = strlen(player_name);
      bHavePlayerName = true;
    }
    else if (!strcmp(argv[curr_arg],"-player_hit_rail"))
      bHavePlayerHitRail = true;
    else if (!strcmp(argv[curr_arg],"-verbose_style2")) {
      bVerbose = true;
      bVerboseStyle2 = true;
    }
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can't specify both -terse and -verbose\n");
    return 3;
  }

  if (bHavePlayerHitRail && !bHavePlayerName) {
    printf("if -player_hit_rail is specified, -player_namename must be also\n");
    return 4;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 5;
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
    num_hands = 0;

    if (bHavePlayerName)
      bPlayerHitRail = false;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (line_no == dbg_line_no)
        dbg = 1;

      if (bDebug)
        printf("line %d %s\n",line_no,line);

      if (!strncmp(line,"Table '",7)) {
        num_hands++;
        table_count = 0;
        finished_count = 0;

        for ( ; ; ) {
          GetLine(fptr,line,&line_len,MAX_LINE_LEN);

          if (feof(fptr))
            break;

          if (!strncmp(line,"*** HOLE CARDS ***",18))
            break;

          if (!strncmp(line,"Seat ",5))
            table_count++;
        }
      }
      else {
        if (Contains(true,
          line,line_len,
          finished,FINISHED_LEN,
          &ix)) {

          finished_count++;

          if (bHavePlayerName) {
            if (Contains(true,
              line,line_len,
              player_name,player_name_len,
              &ix)) {

              bPlayerHitRail = true;
            }
          }
        }
      }
    }

    if (finished_count > 1) {
      if (!bHavePlayerName || (!bHavePlayerHitRail && !bPlayerHitRail) || (bHavePlayerHitRail && bPlayerHitRail)) {
        if (!bVerboseStyle2)
          printf("%d %d %s %3d\n",table_count,table_count - finished_count,filename,num_hands);
        else
          printf("%d %d %s%d.txt\n",table_count,table_count - finished_count,style2(filename),num_hands);
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

#define MAX_STYLE2_LEN 512
static char style2_buf[MAX_STYLE2_LEN];

static char *style2(char *filename)
{
  int n;

  strcpy(style2_buf,filename);

  for (n = strlen(style2_buf) - 1; (n >= 0); n--) {
    if (style2_buf[n] == '\\')
      break;
  }

  n++;

  sprintf(&style2_buf[n],"hand");

  return style2_buf;
}
