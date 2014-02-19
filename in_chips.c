#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: in_chips player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static char *chips(char *line,int ix);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int player_name_ix;
  int player_name_len;
  int ix;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  player_name_ix = 1;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (Contains(true,
      line,line_len,
      argv[player_name_ix],player_name_len,
      &ix)) {

      if (Contains(true,
        line,line_len,
        in_chips,IN_CHIPS_LEN,
        &ix)) {

        printf("%s\n",chips(line,ix));
      }
    }
  }

  fclose(fptr);

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

static char *chips(char *line,int ix)
{
  int n;

  line[ix] = 0;

  for (n = ix - 1; (n >= 0); n--) {
    if (line[n] == '(')
      break;
  }

  return &line[n+1];
}
