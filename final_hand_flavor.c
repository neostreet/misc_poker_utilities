#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 4096
static char line[MAX_LINE_LEN];

static char usage[] = "usage: final_hand_flavor filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static char *get_final_hand_flavor(char *line,int line_len);

int main(int argc,char **argv)
{
  FILE *fptr;
  int filename_len;
  int line_len;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  getcwd(save_dir,_MAX_PATH);

  GetLine(fptr,filename,&filename_len,MAX_FILENAME_LEN);

  if (feof(fptr)) {
    printf("no file names in %s\n",argv[1]);
    fclose(fptr);
    return 3;
  }

  fclose(fptr);

  if ((fptr = fopen(filename,"r")) == NULL) {
    printf(couldnt_open,filename);
    return 4;
  }

  GetLine(fptr,line,&line_len,MAX_LINE_LEN);

  if (feof(fptr)) {
    printf("no lines in %s\n",filename);
    fclose(fptr);
    return 5;
  }

  fclose(fptr);

  printf("%s %s\n",get_final_hand_flavor(line,line_len),save_dir);

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

static char *get_final_hand_flavor(char *line,int line_len)
{
  int m;
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == '(')
      break;
  }

  if (line[n] != '(')
    return line;

  n++;
  m = n;

  for ( ; n < line_len; n++) {
    if (line[n] == ')')
      break;
  }

  if (line[n] != ')')
    return line;

  line[n] = 0;

  return &line[m];
}
