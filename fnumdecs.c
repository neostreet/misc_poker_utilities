#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#define FALSE 0
#define TRUE  1

static char save_dir[_MAX_PATH];

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fnumdecs (-debug) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char summary[] = "*** SUMMARY ***";
static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)
static char folds[] = " folds ";
#define FOLDS_LEN (sizeof (folds) - 1)
static char bets[] = " bets ";
#define BETS_LEN (sizeof (bets) - 1)
static char calls[] = " calls ";
#define CALLS_LEN (sizeof (calls) - 1)
static char raises[] = " raises ";
#define RAISES_LEN (sizeof (raises) - 1)
static char checks[] = " checks ";
#define CHECKS_LEN (sizeof (checks) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(int bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  int bDebug;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;
  int file_no;
  int dbg_file_no;
  int dbg;
  char hole_cards[6];
  int tot_numdecs;
  int numdecs;
  double dwork;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  bDebug = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
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
  dbg_file_no = -1;

  hole_cards[5] = 0;

  tot_numdecs = 0;

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
    numdecs = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(TRUE,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (!strncmp(line,dealt_to,DEALT_TO_LEN)) {
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
        }
        else if (Contains(TRUE,
          line,line_len,
          folds,FOLDS_LEN,
          &ix)) {
          numdecs++;

          break;
        }
        else if (Contains(TRUE,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {
          numdecs++;
        }
        else if (Contains(TRUE,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {
          numdecs++;
        }
        else if (Contains(TRUE,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {
          numdecs++;
        }
        else if (Contains(TRUE,
          line,line_len,
          checks,CHECKS_LEN,
          &ix)) {
          numdecs++;
        }
      }
      else {
        if (!strcmp(line,summary))
          break;
      }
    }

    fclose(fptr);

    if (bDebug)
      printf("%d %s %s\\%s\n",numdecs,hole_cards,save_dir,filename);

    tot_numdecs += numdecs;
  }

  fclose(fptr0);

  if (bDebug)
    putchar(0x0a);

  dwork = (double)tot_numdecs / (double)file_no;

  printf("%10d %3d %5.2lf %s\n",tot_numdecs,file_no,dwork,save_dir);

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

static int Contains(int bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index)
{
  int m;
  int n;
  int tries;
  char chara;

  tries = line_len - string_len + 1;

  if (tries <= 0)
    return FALSE;

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
      return TRUE;
    }
  }

  return FALSE;
}
