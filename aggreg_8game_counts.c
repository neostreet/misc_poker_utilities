#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 4096
static char line[MAX_LINE_LEN];

#define MAX_GAME_NAME_LEN 50
static char game_name[MAX_GAME_NAME_LEN+1];

static char usage[] = "usage: aggreg_8game_counts (-sort) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char *eight_game_names[] = {
  "Triple Draw 2-7 Lowball Limit",
  "Hold'em Limit",
  "Omaha Hi/Lo Limit",
  "Razz Limit",
  "7 Card Stud Limit",
  "7 Card Stud Hi/Lo Limit",
  "Hold'em No Limit",
  "Omaha Pot Limit"
};
#define NUM_8GAME_GAMES (sizeof eight_game_names / sizeof (char *))

static int counts[NUM_8GAME_GAMES];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
void get_8game_name(
  char *line,
  int line_len,
  char *game_name,
  int max_game_name_len);
int get_8game_ix(char *game_name,int *ix);
int elem_compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bSort;
  int retval;
  int ix;
  FILE *fptr;
  int line_len;
  int line_no;
  int ixs[NUM_8GAME_GAMES];

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bSort = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-sort"))
      bSort = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  for (n = 0; n < NUM_8GAME_GAMES; n++) {
    counts[n] = 0;
    ixs[n] = n;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    get_8game_name(line,line_len,game_name,MAX_GAME_NAME_LEN);

    retval = get_8game_ix(game_name,&ix);

    if (retval) {
      printf("get_8game_ix() failed on line %d: %d\n",line_no,retval);
      return 4;
    }

    counts[ix]++;
  }

  fclose(fptr);

  if (bSort)
    qsort(ixs,NUM_8GAME_GAMES,sizeof (int),elem_compare);

  for (n = 0; n < NUM_8GAME_GAMES; n++)
    printf("%10d %s\n",counts[ixs[n]],eight_game_names[ixs[n]]);

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

void get_8game_name(
  char *line,
  int line_len,
  char *game_name,
  int max_game_name_len)
{
  int n;
  int ix;

  if (!Contains(true,
    line,line_len,
    " /cygdrive",10,
    &ix)) {

    ix = line_len;
  }

  for (n = 0; n < ix; n++)
    game_name[n] = line[n];

  game_name[n] = 0;
}

int get_8game_ix(char *game_name,int *ix)
{
  int n;

  for (n = 0; n < NUM_8GAME_GAMES; n++) {
    if (!strcmp(game_name,eight_game_names[n])) {
      *ix = n;
      return 0;
    }
  }

  return 1;
}

int elem_compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;

  ix1 = *(int *)elem1;
  ix2 = *(int *)elem2;

  return counts[ix2] - counts[ix1];
}
