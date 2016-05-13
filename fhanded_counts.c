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

#define MAX_PLAYERS 9

static char usage[] =
"usage: fhanded_counts (-terse) (-verbose) (-debug) (-only_countcount)\n"
"  (-silent) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)

struct count_info {
  int count;
  int starting_balance;
};

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  bool bDebug;
  bool bOnlyCount;
  int only_count;
  bool bSilent;
  int player_name_ix;
  int player_name_len;
  int ix;
  FILE *fptr0;
  int filename_len;
  int num_files;
  int dbg_num_files;
  int dbg;
  FILE *fptr;
  int line_no;
  int line_len;
  int table_count;
  int curr_stack;
  int num_hands;
  struct count_info handed_counts[(MAX_PLAYERS - 1)];
  double handed_count_pct;
  int curr_file_num_hands;

  if ((argc < 2) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  bDebug = false;
  bOnlyCount = false;
  bSilent = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strncmp(argv[curr_arg],"-only_count",11)) {
      bOnlyCount = true;
      sscanf(&argv[curr_arg][11],"%d",&only_count);
    }
    else if (!strcmp(argv[curr_arg],"-silent"))
      bSilent = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can't specify both -terse and -verbose\n");
    return 3;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return (bSilent ? 0 : 4);
  }

  num_files = 0;
  num_hands = 0;

  for (n = 0; n < (MAX_PLAYERS - 1); n++)
    handed_counts[n].count = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    curr_file_num_hands = 0;
    num_files++;

    if (num_files == dbg_num_files)
      dbg = 1;

    line_no = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (!strncmp(line,"Table '",7)) {
        table_count = 0;
        curr_file_num_hands++;

        for ( ; ; ) {
          GetLine(fptr,line,&line_len,MAX_LINE_LEN);

          if (feof(fptr))
            break;

          line_no++;

          if (!strncmp(line,street_marker,STREET_MARKER_LEN))
            break;

          if (!strncmp(line,"Seat ",5)) {
            table_count++;

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

                sscanf(&line[ix+1],"%d",&curr_stack);
              }
            }
          }
        }

        if (bOnlyCount && (table_count < only_count))
          break;

        handed_counts[table_count - 2].count++;

        if (handed_counts[table_count - 2].count == 1)
          handed_counts[table_count - 2].starting_balance = curr_stack;
      }
    }

    fclose(fptr);

    num_hands += curr_file_num_hands;
  }

  fclose(fptr0);

  for (n = 7; (n >= 0); n--) {
    if (!handed_counts[n].count)
      continue;

    if (bOnlyCount && (n + 2 != only_count))
      continue;

    handed_count_pct = (double)handed_counts[n].count / (double)num_hands;

    if (bTerse)
      printf("%d %lf\n",n+2,handed_count_pct);
    else if (!bDebug) {
      if (!bVerbose) {
        printf("%d %lf (%d of %d)\n",n+2,
          handed_count_pct,handed_counts[n].count,num_hands);
      }
      else {
        printf("%d %lf (%d of %d) %d\n",n+2,
          handed_count_pct,handed_counts[n].count,num_hands,
          handed_counts[n].starting_balance);
      }
    }
    else {
      if (!bVerbose) {
        printf("%d %lf (%d of %d) %s\n",n+2,
          handed_count_pct,handed_counts[n].count,num_hands,save_dir);
      }
      else {
        printf("%d %lf (%d of %d) %d %s\n",n+2,
          handed_count_pct,handed_counts[n].count,num_hands,
          handed_counts[n].starting_balance,save_dir);
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
