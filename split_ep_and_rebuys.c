#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char *filenames[] = {
  "ep",
  "rebuys"
};
#define NUM_FILENAMES (sizeof filenames / sizeof (char *))

#define MAX_FILE_NAME_LEN 1024
static char out_filename[MAX_FILE_NAME_LEN];

static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  FILE *out_fptr;
  int line_len;
  int line_no;

  for (n = 0; n < NUM_FILENAMES; n++) {
    if ((fptr = fopen(filenames[n],"r")) == NULL) {
      printf(couldnt_open,filenames[n]);
      return 1;
    }

    line_no = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      sprintf(out_filename,"%c\\%s",'a' + line_no,filenames[n]);

      if ((out_fptr = fopen(out_filename,"w")) == NULL) {
        printf(couldnt_open,out_filename);
        return 2;
      }

      fprintf(out_fptr,"%s\n",line);

      fclose(out_fptr);

      line_no++;
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
