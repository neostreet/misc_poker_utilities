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
"usage: fstarting_balance (-debug) (-min) (-max) (-hand_number)\n"
"  player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bDebug;
  bool bMin;
  bool bMax;
  bool bHandNumber;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;
  int starting_balance;
  int file_no;
  int dbg_file_no;
  int dbg;
  char hole_cards[6];
  int save_starting_balance;

  if ((argc < 3) || (argc > 7)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bMin = false;
  bMax = false;
  bHandNumber = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-min"))
      bMin = true;
    else if (!strcmp(argv[curr_arg],"-max"))
      bMax = true;
    else if (!strcmp(argv[curr_arg],"-hand_number"))
      bHandNumber = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (bMin && bMax) {
    printf("only specify one of -min and -max\n");
    return 3;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  file_no = 0;
  dbg_file_no = -1;

  hole_cards[5] = 0;

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

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
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

          continue;
        }
        else if (!strncmp(line,dealt_to,DEALT_TO_LEN)) {
          for (n = 0; n < line_len; n++) {
            if (line[n] == '[')
              break;
          }

          if (n < line_len) {
            n++;

            for (m = n; m < line_len; m++) {
              if (line[m] == ']')
                break;
            }

            if (m < line_len) {
              for (p = 0; p < 5; p++)
                hole_cards[p] = line[n+p];
            }
          }

          break;
        }
      }
    }

    fclose(fptr);

    if (!bMin && !bMax) {
      if (!bDebug) {
        if (!bHandNumber)
          printf("%d\n",starting_balance);
        else
          printf("%d (%d)\n",starting_balance,file_no);
      }
      else {
        if (!bHandNumber)
          printf("%10d %s %s\\%s\n",starting_balance,hole_cards,save_dir,filename);
        else
          printf("%10d %s %s\\%s (%d)\n",starting_balance,hole_cards,save_dir,filename,file_no);
      }
    }
    else if (bMin) {
      if ((file_no == 1) || (starting_balance < save_starting_balance)) {
        save_starting_balance = starting_balance;
        strcpy(save_filename,filename);
      }
    }
    else {
      if ((file_no == 1) || (starting_balance > save_starting_balance)) {
        save_starting_balance = starting_balance;
        strcpy(save_filename,filename);
      }
    }
  }

  fclose(fptr0);

  if (bMin || bMax) {
    if (!bDebug)
      printf("%d\n",save_starting_balance);
    else
      printf("%10d %s\\%s\n",save_starting_balance,save_dir,save_filename);
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
