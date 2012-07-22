#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: split_hands filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char hand_filename_fmt[] = "hand%d.txt";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr[2];
  int line_len;
  int line_no;
  int hand;
  char buf[20];

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr[0] = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  hand = 0;
  fptr[1] = NULL;

  for ( ; ; ) {
    GetLine(fptr[0],line,&line_len,MAX_LINE_LEN);

    if (feof(fptr[0]))
      break;

    line_no++;

    if (!strncmp(line,"PokerStars ",11)) {
      if (hand)
        fclose(fptr[1]);

      hand++;

      sprintf(buf,hand_filename_fmt,hand);

      if ((fptr[1] = fopen(buf,"w")) == NULL) {
        printf(couldnt_open,buf);
        return 3;
      }

      fprintf(fptr[1],"%s\n",line);
    }
    else if (hand)
      fprintf(fptr[1],"%s\n",line);
  }

  fclose(fptr[0]);

  if (hand) {
    fclose(fptr[1]);

    if ((fptr[0] = fopen("hands.lst","w")) == NULL) {
      printf(couldnt_open,buf);
      return 4;
    }

    if ((fptr[1] = fopen("hands.ls0","w")) == NULL) {
      fclose(fptr[0]);
      printf(couldnt_open,buf);
      return 5;
    }

    for (n = 1; n <= hand; n++) {
      fprintf(fptr[0],hand_filename_fmt,n);
      fputc(0x0a,fptr[0]);
      fprintf(fptr[1],hand_filename_fmt,hand - (n - 1));
      fputc(0x0a,fptr[1]);
    }

    fclose(fptr[0]);
    fclose(fptr[1]);
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

    if ((chara == 0xef) || (chara == 0xbb) || (chara == 0xbf))
      continue;

    if (local_line_len < maxllen - 1)
      line[local_line_len++] = (char)chara;
  }

  line[local_line_len] = 0;
  *line_len = local_line_len;
}
