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
static char table_line[MAX_LINE_LEN];

struct big_blind_struct {
  int big_blind;
  int table_size;
};

#define MAX_BIG_BLIND_STRUCTS 10
static struct big_blind_struct big_blind_structs[MAX_BIG_BLIND_STRUCTS];

static char usage[] =
"usage: fbig_blind (-debug) (-verbose) (-per_file) (-get_date_from_cwd)\n"
"  (-get_date_from_filename) (-table_size) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);
static int get_big_blind(
  char *line,
  int line_len,
  int *big_blind_ptr
);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bPerFile;
  bool bGetDate;
  bool bGetDateFromCwd;
  bool bGetDateFromFilename;
  bool bTableSize;
  char *date_string;
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_no;
  int line_len;
  int table_line_len;
  int ix;
  int retval;
  int num_big_blind_structs;
  int curr_big_blind;
  int curr_table_size;

  if ((argc < 2) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bPerFile = false;
  bGetDate = false;
  bGetDateFromCwd = false;
  bGetDateFromFilename = false;
  bTableSize = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-per_file"))
      bPerFile = true;
    else if (!strcmp(argv[curr_arg],"-get_date_from_cwd")) {
      bGetDate = true;
      bGetDateFromCwd = true;
    }
    else if (!strcmp(argv[curr_arg],"-get_date_from_filename")) {
      bGetDate = true;
      bGetDateFromFilename = true;
    }
    else if (!strcmp(argv[curr_arg],"-table_size"))
      bTableSize = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bGetDateFromCwd && bGetDateFromFilename) {
    printf("can't specify both -get_date_from_cwd and -get_date_from_filename\n");
    return 3;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  if (bGetDateFromCwd) {
    getcwd(save_dir,_MAX_PATH);

    retval = get_date_from_path(save_dir,'/',2,&date_string);

    if (retval) {
      printf("get_date_from_path() failed: %d\n",retval);
      return 5;
    }
  }

  num_files = 0;
  num_big_blind_structs = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if (bGetDateFromFilename) {
      retval = get_date_from_path(filename,'\\',3,&date_string);

      if (retval) {
        printf("get_date_from_path() failed: %d\n",retval);
        return 6;
      }
    }

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    num_files++;
    line_no = 0;

    if (bPerFile)
      num_big_blind_structs = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (!strncmp(line,"PokerStars ",11)) {
        retval = get_big_blind(line,line_len,&curr_big_blind);

        if (retval) {
          printf("%s: get_big_blind() failed on line %d: %d\n",filename,line_len,retval);
          return 7;
        }

        if (bDebug)
          printf("%d\n",curr_big_blind);

        if (bTableSize) {
          GetLine(fptr,table_line,&table_line_len,MAX_LINE_LEN);

          if (feof(fptr)) {
            printf("%s: couldn't get table line after line %d\n",filename,line_no);
            return 8;
          }

          line_no++;

          if (Contains(true,
            table_line,table_line_len,
            "-max",4,
            &ix)) {

            line[ix] = 0;
            sscanf(&table_line[ix-1],"%d",&curr_table_size);
          }
          else
            curr_table_size = 0;
        }

        for (n = 0; n < num_big_blind_structs; n++) {
          if (!bTableSize) {
            if (big_blind_structs[n].big_blind == curr_big_blind)
              break;
          }
          else {
            if ((big_blind_structs[n].big_blind == curr_big_blind) &&
                (big_blind_structs[n].table_size == curr_table_size))
              break;
          }
        }

        if (n == num_big_blind_structs) {
          if (num_big_blind_structs == MAX_BIG_BLIND_STRUCTS) {
            printf("MAX_BIG_BLIND_STRUCTS value of %d exceeded\n",
              MAX_BIG_BLIND_STRUCTS);
            return 9;
          }

          big_blind_structs[num_big_blind_structs].big_blind = curr_big_blind;

          if (bTableSize)
            big_blind_structs[num_big_blind_structs].table_size = curr_table_size;

          num_big_blind_structs++;

          if (!bDebug) {
            if (!bVerbose) {
              if (bPerFile || num_big_blind_structs == 1) {
                if (!bGetDate) {
                  if (!bTableSize)
                    printf("%d\n",curr_big_blind);
                  else
                    printf("%d %d\n",curr_big_blind,curr_table_size);
                }
                else {
                  if (!bTableSize)
                    printf("%d\t%s\n",curr_big_blind,date_string);
                  else
                    printf("%d\t%d\t%s\n",curr_big_blind,curr_table_size,date_string);
                }
              }
              else {
                if (!bGetDate) {
                  if (!bTableSize)
                    printf("%d (%d)\n",curr_big_blind,num_big_blind_structs);
                  else
                    printf("%d %d (%d)\n",curr_big_blind,curr_table_size,num_big_blind_structs);
                }
                else {
                  if (!bTableSize) {
                    printf("%d\t(%d)\t%s\n",curr_big_blind,num_big_blind_structs,
                      date_string);
                  }
                  else {
                    printf("%d\t%d\t(%d)\t%s\n",curr_big_blind,curr_table_size,num_big_blind_structs,
                      date_string);
                  }
                }
              }
            }
            else {
              if (bPerFile || num_big_blind_structs == 1) {
                if (!bGetDate) {
                  if (!bTableSize)
                    printf("%d %s %s\n",curr_big_blind,filename,line);
                  else
                    printf("%d %d %s %s\n",curr_big_blind,curr_table_size,filename,line);
                }
                else {
                  if (!bTableSize)
                    printf("%d\t%s\t%s\t%s\n",curr_big_blind,date_string,filename,line);
                  else
                    printf("%d\t%d\t%s\t%s\t%s\n",curr_big_blind,curr_table_size,date_string,filename,line);
                }
              }
              else {
                if (!bGetDate) {
                  if (!bTableSize) {
                    printf("%d (%d) %s %s\n",curr_big_blind,num_big_blind_structs,
                      filename,line);
                  }
                  else {
                    printf("%d %d (%d) %s %s\n",curr_big_blind,curr_table_size,num_big_blind_structs,
                      filename,line);
                  }
                }
                else {
                  if (!bTableSize) {
                    printf("%d\t(%d)\t%s\t%s\t%s\n",curr_big_blind,num_big_blind_structs,
                      date_string,filename,line);
                  }
                  else {
                    printf("%d\t%d\t(%d)\t%s\t%s\t%s\n",curr_big_blind,curr_table_size,num_big_blind_structs,
                      date_string,filename,line);
                  }
                }
              }
            }
          }
        }
      }
    }

    fclose(fptr);
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

static int get_big_blind(
  char *line,
  int line_len,
  int *big_blind_ptr
)
{
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == ')')
      break;
  }

  if (n == line_len)
    return 1;

  for (n--; (n >= 0); n--) {
    if (line[n] == '/')
      break;
  }

  if (n < 0)
    return 2;

  n++;

  sscanf(&line[n],"%d",big_blind_ptr);

  return 0;
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
