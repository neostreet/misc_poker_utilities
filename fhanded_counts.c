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
"usage: fhanded_counts (-terse) (-verbose) (-debug) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  bool bDebug;
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_len;
  int table_count;
  int num_hands;
  int handed_counts[5];
  double handed_count_pct;
  int curr_file_num_hands;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
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

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  num_files = 0;
  num_hands = 0;

  for (n = 0; n < 5; n++)
    handed_counts[n] = 0;

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

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (!strncmp(line,"Table '",7)) {
        table_count = 0;
        curr_file_num_hands++;

        for ( ; ; ) {
          GetLine(fptr,line,&line_len,MAX_LINE_LEN);

          if (feof(fptr))
            break;

          if (!strncmp(line,street_marker,STREET_MARKER_LEN))
            break;

          if (!strncmp(line,"Seat ",5))
            table_count++;
        }

        handed_counts[table_count - 2]++;
      }
    }

    fclose(fptr);

    num_hands += curr_file_num_hands;
  }

  fclose(fptr0);

  for (n = 4; (n >= 0); n--) {
    if (!handed_counts[n])
      continue;

    handed_count_pct = (double)handed_counts[n] / (double)num_hands;

    if (bTerse)
      printf("%d %lf\n",n+2,handed_count_pct);
    else if (!bDebug)
      printf("%d %lf (%d of %d)\n",n+2,handed_count_pct,handed_counts[n],num_hands);
    else
      printf("%d %lf (%d of %d) %s\n",n+2,handed_count_pct,handed_counts[n],num_hands,save_dir);
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
