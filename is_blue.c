#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: is_blue (-starting_amountstarting_amount) (-terse) (-verbose)\n"
"  (-not) (-date_first) (-only_blue) (-star_ge_amountamount) (-grand_slam)\n"
"  (-blue_amount_first) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

#define DATE_LEN 11

int main(int argc,char **argv)
{
  int curr_arg;
  int starting_amount;
  bool bTerse;
  bool bVerbose;
  bool bNot;
  bool bDateFirst;
  bool bOnlyBlue;
  bool bGrandSlam;
  bool bBlueAmountFirst;
  int star_ge_amount;
  int balance;
  FILE *fptr;
  int line_len;
  int line_no;
  int blue_count;
  int work;
  int max;
  char date[DATE_LEN];
  int intervening_profits;
  int intervening_losses;

  if ((argc < 2) || (argc > 11)) {
    printf(usage);
    return 1;
  }

  starting_amount = 0;
  bTerse = false;
  bVerbose = false;
  bNot = false;
  bDateFirst = false;
  bOnlyBlue = false;
  bGrandSlam = false;
  bBlueAmountFirst = false;
  star_ge_amount = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strncmp(argv[curr_arg],"-starting_amount",16))
      sscanf(&argv[curr_arg][16],"%d",&starting_amount);
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else if (!strcmp(argv[curr_arg],"-date_first"))
      bDateFirst = true;
    else if (!strcmp(argv[curr_arg],"-only_blue"))
      bOnlyBlue = true;
    else if (!strncmp(argv[curr_arg],"-star_ge_amount",15))
      sscanf(&argv[curr_arg][15],"%d",&star_ge_amount);
    else if (!strcmp(argv[curr_arg],"-grand_slam"))
      bGrandSlam = true;
    else if (!strcmp(argv[curr_arg],"-blue_amount_first"))
      bBlueAmountFirst = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can only specify one of -terse and -verbose\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;
  blue_count = 0;
  balance = starting_amount;
  max = starting_amount;
  intervening_profits = 0;
  intervening_losses = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d %s",&work,date);

    balance += work;

    if (balance >= max) {
      blue_count++;
      max = balance;

      if (bTerse) {
        if (!bOnlyBlue) {
          if (!bNot)
            printf("1\n");
          else
            printf("0\n");
        }
        else
          printf("%d\n",balance);
      }
      else if (bVerbose) {
        if (!bNot) {
          if (!bOnlyBlue) {
            if (!bGrandSlam)
              printf("1 %d %s\n",balance,line);
            else {
              if (!intervening_profits && (intervening_losses >= 4))
                printf("1 1 %d %s\n",balance,line);
              else
                printf("1 0 %d %s\n",balance,line);
            }
          }
          else {
            if ((star_ge_amount == -1) || (work < star_ge_amount)) {
              if (!bBlueAmountFirst)
                printf("%s %d\n",line,balance);
              else
                printf("%d %s\n",balance,line);
            }
            else {
              if (!bBlueAmountFirst)
                printf("%s * %d\n",line,balance);
              else
                printf("%d * %s\n",balance,line);
            }
          }
        }
        else if (!bOnlyBlue)
          printf("0 %d %s\n",balance,line);
      }
      else {
        if (!bNot) {
          if (!bDateFirst)
            printf("1\t%s\n",date);
          else
            printf("%s\t1\n",date);
        }
        else {
          if (!bDateFirst)
            printf("0\t%s\n",date);
          else
            printf("%s\t0\n",date);
        }
      }

      intervening_profits = 0;
      intervening_losses = 0;
    }
    else {
      if (work > 0)
        intervening_profits++;

      if (work < 0)
        intervening_losses++;

      if (bTerse) {
        if (!bOnlyBlue) {
          if (!bNot)
            printf("0\n");
          else
            printf("1\n");
        }
      }
      else if (bVerbose) {
        if (!bNot) {
          if (!bOnlyBlue)
            printf("0 %d %s\n",balance,line);
        }
        else {
          if (!bOnlyBlue)
            printf("1 %d %s\n",balance,line);
          else {
            if ((star_ge_amount == -1) || (work < star_ge_amount))
              printf("%s %d\n",line,balance);
            else
              printf("%s * %d\n",line,balance);
          }
        }
      }
      else {
        if (!bNot) {
          if (!bDateFirst)
            printf("0\t%s\n",date);
          else
            printf("%s\t0\n",date);
        }
        else {
          if (!bDateFirst)
            printf("1\t%s\n",date);
          else
            printf("%s\t1\n",date);
        }
      }
    }

    line_no++;
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
