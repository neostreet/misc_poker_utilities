#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 8192
static char line[MAX_LINE_LEN];
static char filename[MAX_LINE_LEN];

static char usage[] = "usage: split_hand_history filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char hand_list_file[] = "hands.lst";
static char hand_filename_fmt[] = "hand%d.txt";

static char pokerstars[] = "PokerStars";
#define POKERSTARS_LEN (sizeof (pokerstars) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr[3];
  int line_len;
  int line_no;
  int hand;
  int ix;
  char buf[20];

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr[0] = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  if ((fptr[1] = fopen(hand_list_file,"w")) == NULL) {
    printf(couldnt_open,hand_list_file);
    return 3;
  }

  line_no = 0;
  hand = 0;
  fptr[2] = NULL;

  for ( ; ; ) {
    GetLine(fptr[0],line,&line_len,MAX_LINE_LEN);

    if (feof(fptr[0]))
      break;

    line_no++;

    if (Contains(true,
      line,line_len,
      pokerstars,POKERSTARS_LEN,
      &ix)) {

      hand++;

      if (fptr[2]) {
        fclose(fptr[2]);
        fptr[2] = NULL;
      }

      sprintf(buf,hand_filename_fmt,hand);
      fprintf(fptr[1],"%s\n",buf);

      if ((fptr[2] = fopen(buf,"w")) == NULL) {
        printf(couldnt_open,buf);
        return 4;
      }

      fprintf(fptr[2],"%s\n",line);
    }
    else if (fptr[2])
      fprintf(fptr[2],"%s\n",line);
  }

  if (fptr[2])
    fclose(fptr[2]);

  fclose(fptr[1]);
  fclose(fptr[0]);

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
