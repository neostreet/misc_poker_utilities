#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_MILLION_BOUNDARIES 500
static int crossings[MAX_MILLION_BOUNDARIES];

static char usage[] =
"usage: million_boundary_crossings (-verbose) (-terse) (-ge_valval) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bVerbose;
  bool bTerse;
  int ge_val;
  FILE *fptr;
  int line_len;
  int line_no;
  char date_str[20];
  int starting_balance;
  int ending_balance;
  int starting_million;
  int ending_million;
  int dbg_million;
  int dbg;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bTerse = false;
  ge_val = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strncmp(argv[curr_arg],"-ge_val",7))
      sscanf(&argv[curr_arg][7],"%d",&ge_val);
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bVerbose && bTerse) {
    printf("can't specify both -verbose and -terse\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  for (n = 0; n < MAX_MILLION_BOUNDARIES; n++)
    crossings[n] = 0;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%s\t%d\t%d",&date_str[0],&starting_balance,&ending_balance);

    starting_million = starting_balance / 1000000;
    ending_million = ending_balance / 1000000;

    if (ending_million >= MAX_MILLION_BOUNDARIES) {
      printf("ending_million (%d) > MAX_MILLION_BOUNDARIES (%d)\n",
        ending_million,MAX_MILLION_BOUNDARIES);
      fclose(fptr);
      return 5;
    }

    if (starting_million != ending_million) {
      if (((starting_million < dbg_million) && (ending_million >= dbg_million)) ||
          ((starting_million >= dbg_million) && (ending_million < dbg_million)))
        dbg = 1;

      if (starting_million < ending_million) {
        for (starting_million++; starting_million <= ending_million; starting_million++) {
          if (bVerbose) {
            if ((ge_val == -1) || (starting_million >= ge_val))
              printf("%s %d\n",date_str,starting_million);
          }
          else if (bTerse) {
            if ((ge_val == -1) || (starting_million >= ge_val))
              printf("%d\n",starting_million);
          }
          else
            crossings[starting_million]++;
        }
      }
      else {
        for (starting_million--; starting_million >= ending_million; starting_million--) {
          if (bVerbose) {
            if ((ge_val == -1) || (starting_million >= ge_val))
              printf("%s %d\n",date_str,starting_million);
          }
          else if (bTerse) {
            if ((ge_val == -1) || (starting_million >= ge_val))
              printf("%d\n",starting_million);
          }
          else
            crossings[starting_million]++;
        }
      }
    }
  }

  fclose(fptr);

  if (!bVerbose) {
    for (n = MAX_MILLION_BOUNDARIES - 1; (n > 0); n--) {
      if (crossings[n])
        break;
    }

    for (m = 1; m <= n; m++)
      printf("%2d %8d\n",crossings[m],m * 1000000);
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
