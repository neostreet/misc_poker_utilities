#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: gen_style_and_flavor filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char headings[] =
  "style flavor\n";

static char *poker_styles[] = {
  "Cash ",
  "SNG  ",
  "MTT  ",
  "Sp&Go",
  "KO   "
};
#define NUM_POKER_STYLES (sizeof poker_styles / sizeof (char *))

static char *poker_flavors[] = {
  "PLHE  ",
  "PLO   ",
  "7Stud ",
  "NLHE  ",
  "L5draw",
  "NL27Lo",
  "PL27Lo",
  "LHE   ",
  "L27Lo ",
  "8-Game",
  "HORSE "
};
#define NUM_POKER_FLAVORS (sizeof poker_flavors / sizeof (char *))

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int style;
  int flavor;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  printf("%s\n",headings);

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %d",&style,&flavor);

    if (style >= NUM_POKER_STYLES) {
      printf("invalid style %d on line %d\n",style,line_no);
      return 3;
    }

    if (flavor >= NUM_POKER_FLAVORS) {
      printf("invalid flavor %d on line %d\n",flavor,line_no);
      return 4;
    }

    printf("%s %s\n",poker_styles[style],poker_flavors[flavor]);
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
