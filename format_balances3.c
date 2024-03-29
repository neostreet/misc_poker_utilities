#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define BUF_LEN 20

static char usage[] = "usage: format_balances3 year filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static char *format_balance(int bal,char *buf,int buf_len);
static void format_itm_pct(char *line);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int bal;
  char buf[BUF_LEN+1];

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  line_no = 0;

  printf("PokerStars play money stats\n\n");

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    switch (line_no) {
      case 2:
        sscanf(&line[11],"%d",&bal);
        printf("delta: %s\n",format_balance(bal,buf,BUF_LEN));
        break;
      case 5:
        sscanf(&line[0],"%d",&bal);
        printf("%s balance: %s\n",argv[1],
          format_balance(bal,buf,BUF_LEN));
        break;
      case 8:
        sscanf(&line[0],"%d",&bal);
        printf("blue distance: %s\n",format_balance(bal,buf,BUF_LEN));
        break;
      case 11:
        sscanf(&line[0],"%d",&bal);
        printf("balance: %s\n",format_balance(bal,buf,BUF_LEN));
        break;
      case 13:
        format_itm_pct(line);
        break;
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

static char *format_balance(int bal,char *buf,int buf_len)
{
  int m;
  int n;
  int len;

  sprintf(buf,"%d",bal);
  len = strlen(buf);

  buf[buf_len] = 0;
  n = 0;
  m = buf_len;

  for ( ; ; ) {
    buf[--m] = buf[len - 1 - n];

    n++;

    if (n == len)
      break;

    if (!(n % 3)) {
      if (buf[len - 1 - n] != '-')
        buf[--m] = ',';
    }
  }

  buf[--m] = '$';

  return &buf[m];
}

static void format_itm_pct(char *line)
{
  double itm_pct;
  int itm;
  int count;

  sscanf(line,"%lf\t%d\t%d",&itm_pct,&itm,&count);
  printf("MTT NLHE ITM pct: %5.2lf (%d of %d)\n",itm_pct * (double)100,itm,count);
}
