#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];
static char prev_line[MAX_LINE_LEN];

static char usage[] =
"usage: repeated_hands (-debug) (-dont_print_prev) (-streak_counts) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char fmt_str[] = "%s\n";
static char fmt_str2[] = "%d %s\n";
static char fmt_str3[] = "DEBUG: %d %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bDontPrintPrev;
  bool bStreakCounts;
  FILE *fptr;
  int line_len;
  int line_no;
  int dbg_line_no;
  int dbg;
  char prev_hand[3];
  char curr_hand[3];
  bool bPrintedPrev;
  int streak_count;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bDontPrintPrev = false;
  bStreakCounts = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-dont_print_prev"))
      bDontPrintPrev = true;
    else if (!strcmp(argv[curr_arg],"-streak_counts"))
      bStreakCounts = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  line_no = 0;
  dbg_line_no = 0;
  bPrintedPrev = false;

  if (bStreakCounts)
    streak_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (line_no == dbg_line_no)
      dbg = 1;

    for (n = 0; n < 3; n++)
      curr_hand[n] = line[n];

    if (line_no > 1) {
      for (n = 0; n < 3; n++) {
        if (curr_hand[n] != prev_hand[n])
          break;
      }

      if (n == 3) {
        if (bStreakCounts) {
          if (!streak_count) {
            streak_count++;

            if (bDebug)
              printf(fmt_str3,streak_count,line);
          }
          else {
            if (streak_count) {
              printf(fmt_str2,streak_count+1,prev_line);
              streak_count = 0;
            }
          }
        }
        else {
          if (!bDontPrintPrev && !bPrintedPrev) {
            printf(fmt_str,prev_line);
            bPrintedPrev = true;
          }

          printf(fmt_str,line);
        }
      }
      else {
        if (bStreakCounts) {
          if (streak_count) {
            printf(fmt_str2,streak_count+1,prev_line);
            streak_count = 0;
          }
        }
        else
          bPrintedPrev = false;
      }
    }

    strcpy(prev_line,line);

    for (n = 0; n < 3; n++)
      prev_hand[n] = curr_hand[n];
  }

  fclose(fptr);

  if (streak_count)
    printf(fmt_str2,streak_count+1,prev_line);

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
