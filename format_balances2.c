#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define BUF_LEN 20

static char usage[] =
"usage: format_balances balance_str1 balance_str2 filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static char *format_balance(int bal,char *buf,int buf_len);
static char fmt_str[] = "%s: %s\n";

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int bal;
  char buf[BUF_LEN+1];

  if (argc != 4) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[3],"r")) == NULL) {
    printf(couldnt_open,argv[3]);
    return 2;
  }

  line_no = 0;

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
        printf(fmt_str,argv[1],
          format_balance(bal,buf,BUF_LEN));
        break;
      case 7:
        sscanf(&line[0],"%d",&bal);
        printf(fmt_str,argv[2],
          format_balance(bal,buf,BUF_LEN));
        break;
      case 9:
        sscanf(&line[0],"%d",&bal);
        printf("balance: %s\n",format_balance(bal,buf,BUF_LEN));
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
