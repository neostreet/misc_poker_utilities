#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: fpots (-debug) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char you_were_dealt_str[] =
  "During current Hold'em session you were dealt ";
#define YOU_WERE_DEALT_LEN (sizeof (you_were_dealt_str) - 1)

static char pots_won_at_showdown_str[] =
  " Pots won at showdown - ";
#define POTS_WON_AT_SHOWDOWN_LEN (sizeof (pots_won_at_showdown_str) - 1)

static char pots_won_without_showdown_str[] =
  " Pots won without showdown - ";
#define POTS_WON_WITHOUT_SHOWDOWN_LEN (sizeof (pots_won_without_showdown_str) - 1)

void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int bDebug;
  int curr_arg;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int linelen;
  int total_hands;
  int pots_won_without_showdown;
  int pots_won_at_showdown;
  int pots_won;
  double win_pct;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = TRUE;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    pots_won_without_showdown = 0;
    pots_won_at_showdown = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&linelen,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (!strncmp(line,you_were_dealt_str,YOU_WERE_DEALT_LEN)) {
        sscanf(&line[YOU_WERE_DEALT_LEN],"%d",&total_hands);
        continue;
      }

      if (!strncmp(line,pots_won_without_showdown_str,POTS_WON_WITHOUT_SHOWDOWN_LEN)) {
        sscanf(&line[POTS_WON_WITHOUT_SHOWDOWN_LEN],"%d",&pots_won_without_showdown);
        continue;
      }

      if (!strncmp(line,pots_won_at_showdown_str,POTS_WON_AT_SHOWDOWN_LEN)) {
        sscanf(&line[POTS_WON_AT_SHOWDOWN_LEN],"%d",&pots_won_at_showdown);
        continue;
      }
    }

    pots_won = pots_won_without_showdown + pots_won_at_showdown;

    if (!bDebug)
      printf("%3d %s\n",pots_won,filename);
    else {
      win_pct = (double)pots_won / (double)total_hands * (double)100;

      printf("%3d %3d %3d %3d %s\n",
        total_hands,
        pots_won_without_showdown,
        pots_won_at_showdown,
        pots_won,
        filename);
    }

    fclose(fptr);
  }

  fclose(fptr0);

  return 0;
}

void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
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
