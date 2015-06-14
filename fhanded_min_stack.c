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
"usage: fhanded_min_stack (-debug) (-verbose)\n"
"  player_name handed_count filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  int player_name_ix;
  int player_name_len;
  int handed_count;
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_len;
  int table_count;
  int stack;
  int min_stack;
  int min_stack_handed_count;
  int ix;
  int curr_file_num_hands;

  if ((argc < 4) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 3) {
    printf(usage);
    return 2;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  sscanf(argv[curr_arg++],"%d",&handed_count);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  num_files = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    min_stack = -1;
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

          if (!strncmp(line,"Seat ",5)) {
            table_count++;

            if (Contains(true,
              line,line_len,
              argv[player_name_ix],player_name_len,
              &ix)) {

              sscanf(&line[ix + player_name_len + 2],"%d",&stack);
            }
          }
        }

        if ((handed_count == -1) || (table_count == handed_count)) {
          if ((min_stack == -1) || (stack < min_stack)) {
            min_stack = stack;

            if (handed_count == -1)
              min_stack_handed_count = table_count;
          }

          if (bVerbose) {
            if (!bDebug)
              printf("%d %d\n",stack,curr_file_num_hands);
            else
              printf("%d %s/%s %d\n",stack,save_dir,filename,curr_file_num_hands);
          }
        }
      }
    }

    fclose(fptr);

    if (min_stack != -1) {
      if (!bDebug) {
        if (handed_count != -1)
          printf("%d\n",min_stack);
        else
          printf("%d (%d)\n",min_stack,min_stack_handed_count);
      }
      else {
        if (handed_count != -1)
          printf("%d %s/%s\n",min_stack,save_dir,filename);
        else
          printf("%d (%d) %s/%s\n",min_stack,min_stack_handed_count,save_dir,filename);
      }
    }
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
