#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fsession_length3 (-terse) (-debug) (-gelength) (-ltlength)\n"
"  (-filename_only) (-get_date_from_filename) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char summary[] = "*** SUMMARY ***";
#define SUMMARY_LEN (sizeof (summary) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bTerse;
  bool bDebug;
  bool bGeLength;
  int ge_length;
  bool bLtLength;
  int lt_length;
  bool bFilenameOnly;
  bool bGetDateFromFilename;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  char *date_string;
  int dbg_line_no;
  int ix;
  int file_no;
  int dbg_file_no;
  int num_hands;
  int dbg;
  bool bSkipping;

  if ((argc < 3) || (argc > 9)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bDebug = false;
  bGeLength = false;
  bLtLength = false;
  bFilenameOnly = false;
  bGetDateFromFilename = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strncmp(argv[curr_arg],"-ge",3) &&
      (argv[curr_arg][3] >= '0') && (argv[curr_arg][3] <= '9')) {
      bGeLength = true;
      sscanf(&argv[curr_arg][3],"%d",&ge_length);
    }
    else if (!strncmp(argv[curr_arg],"-lt",3)) {
      bLtLength = true;
      sscanf(&argv[curr_arg][3],"%d",&lt_length);
    }
    else if (!strcmp(argv[curr_arg],"-filename_only"))
      bFilenameOnly = true;
    else if (!strcmp(argv[curr_arg],"-get_date_from_filename"))
      bGetDateFromFilename = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (bGeLength && bLtLength) {
    printf("can't specify both -gelength and -ltlength\n");
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

    if (bGetDateFromFilename) {
      retval = get_date_from_path(filename,'\\',3,&date_string);

      if (retval) {
        printf("get_date_from_path() on %s failed: %d\n",filename,retval);
        continue;
      }
    }

    line_no = 0;
    bSkipping = false;
    num_hands = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (line_no == dbg_line_no)
        dbg = 1;

      if (bDebug)
        printf("line %d %s\n",line_no,line);

      if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (Contains(true,
          line,line_len,
          in_chips,IN_CHIPS_LEN,
          &ix)) {

          num_hands++;
          bSkipping = false;
        }
      }
      else if (bSkipping)
        ;
      else {
        if (!strncmp(line,summary,SUMMARY_LEN)) {
          if (bDebug)
            printf("line %d SUMMARY line detected; skipping\n",line_no);

          bSkipping = true;
        }
      }
    }

    fclose(fptr);

    if (!bGeLength || (num_hands >= ge_length)) {
      if (!bLtLength || (num_hands < lt_length)) {
        if (bTerse)
          printf("%3d\n",num_hands);
        else if (bFilenameOnly)
          printf("%s\n",filename);
        else {
          if (!bGetDateFromFilename)
            printf("%3d %s\n",num_hands,filename);
          else
            printf("%d\t%s\n",num_hands,date_string);
        }
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

static char sql_date_string[11];

static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr)
{
  int n;
  int len;
  int slash_count;

  len = strlen(path);
  slash_count = 0;

  for (n = len - 1; (n >= 0); n--) {
    if (path[n] == slash_char) {
      slash_count++;

      if (slash_count == num_slashes)
        break;
    }
  }

  if (slash_count != num_slashes)
    return 1;

  if (path[n+5] != slash_char)
    return 2;

  strncpy(sql_date_string,&path[n+1],4);
  sql_date_string[4] = '-';
  strncpy(&sql_date_string[5],&path[n+6],2);
  sql_date_string[7] = '-';
  strncpy(&sql_date_string[8],&path[n+8],2);
  sql_date_string[10] = 0;

  *date_string_ptr = sql_date_string;

  return 0;
}
