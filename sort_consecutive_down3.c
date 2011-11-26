#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

#define SORT_NO_SORT                     0
#define SORT_BY_LOSS                     1
#define SORT_BY_SESSIONS                 2
#define SORT_BY_AVERAGE                  3
#define SORT_BY_LOSS_PCT                 4

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: sort_consecutive_down3 (-descending)\n"
"  [no_sort | by_loss | by_sessions | by_average | by_loss_pct] filename\n";
static char couldnt_open[] = "couldn't open %s\n";

#define MYSQL_DATE_STR_LEN 10

struct loss_struct {
  char loss_start_date[MYSQL_DATE_STR_LEN+1];
  int starting_amount;
  int loss;
  int loss_num_consecutive_sessions;
  double loss_average;
  double loss_pct;
};

static int bDescending;
static int sort;
static struct loss_struct *losses;

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
int elem_compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  FILE *fptr;
  int line_len;
  int prev_delta;
  int curr_delta;
  int curr_starting_amount;
  char curr_date_str[MYSQL_DATE_STR_LEN+1];
  int nobs;
  int *ixs;
  int curr_ix1;
  int curr_ix2;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDescending = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-descending"))
      bDescending = TRUE;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (!strcmp(argv[curr_arg],"no_sort"))
    sort = SORT_NO_SORT;
  else if (!strcmp(argv[curr_arg],"by_loss"))
    sort = SORT_BY_LOSS;
  else if (!strcmp(argv[curr_arg],"by_sessions"))
    sort = SORT_BY_SESSIONS;
  else if (!strcmp(argv[curr_arg],"by_average"))
    sort = SORT_BY_AVERAGE;
  else if (!strcmp(argv[curr_arg],"by_loss_pct"))
    sort = SORT_BY_LOSS_PCT;
  else {
    printf(usage);
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 4;
  }

  prev_delta = 1;
  nobs = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d",&curr_delta);

    if (curr_delta < 0) {
      if (prev_delta > 0)
        nobs++;
    }

    prev_delta = curr_delta;
  }

  if ((losses = (struct loss_struct *)malloc(nobs *
    sizeof (struct loss_struct))) == NULL) {
    printf("couldn't malloc %d loss structs\n",nobs);
    return 5;
  }

  if ((ixs = (int *)malloc(nobs * sizeof (int))) == NULL) {
    printf("couldn't malloc %d ints\n",nobs);
    return 6;
  }

  fseek(fptr,0L,SEEK_SET);

  curr_ix1 = 0;
  prev_delta = 1;
  nobs = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d %d %s",&curr_delta,&curr_starting_amount,curr_date_str);

    if (curr_delta < 0) {
      if (prev_delta > 0) {
        curr_ix2 = nobs;

        losses[curr_ix2].starting_amount = curr_starting_amount;
        strcpy(losses[curr_ix2].loss_start_date,curr_date_str);
        losses[curr_ix2].loss = curr_delta;
        losses[curr_ix2].loss_num_consecutive_sessions = 1;

        nobs++;
      }
      else {
        losses[curr_ix2].loss += curr_delta;
        losses[curr_ix2].loss_num_consecutive_sessions += 1;
      }
    }

    prev_delta = curr_delta;

    curr_ix1++;
  }

  for (n = 0; n < nobs; n++) {
    losses[n].loss_average = (double)losses[n].loss /
      (double)losses[n].loss_num_consecutive_sessions;
    losses[n].loss_pct = (double)losses[n].loss /
      (double)losses[n].starting_amount * (double)100;
    ixs[n] = n;
  }

  if (sort != SORT_NO_SORT)
    qsort(ixs,nobs,sizeof (int),elem_compare);

  for (n = 0; n < nobs; n++) {
    printf("%s: %3d %10d %13.2lf %6.2lf\n",
      losses[ixs[n]].loss_start_date,
      losses[ixs[n]].loss_num_consecutive_sessions,
      losses[ixs[n]].loss,
      losses[ixs[n]].loss_average,
      losses[ixs[n]].loss_pct);
  }

  fclose(fptr);

  free(ixs);
  free(losses);

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

int elem_compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;

  if (!bDescending) {
    ix1 = *(int *)elem1;
    ix2 = *(int *)elem2;
  }
  else {
    ix1 = *(int *)elem2;
    ix2 = *(int *)elem1;
  }

  switch(sort) {
    case SORT_BY_LOSS:
      if (losses[ix1].loss == losses[ix2].loss)
        return losses[ix2].loss_num_consecutive_sessions -
          losses[ix1].loss_num_consecutive_sessions;
      else
        return losses[ix1].loss - losses[ix2].loss;

      break;
    case SORT_BY_SESSIONS:
      if (losses[ix1].loss_num_consecutive_sessions ==
          losses[ix2].loss_num_consecutive_sessions)
        return losses[ix1].loss - losses[ix2].loss;
      else
        return losses[ix2].loss_num_consecutive_sessions -
          losses[ix1].loss_num_consecutive_sessions;

      break;
    case SORT_BY_AVERAGE:
      return (int)(losses[ix1].loss_average - losses[ix2].loss_average);

      break;
    case SORT_BY_LOSS_PCT:
      return (int)(losses[ix1].loss_pct - losses[ix2].loss_pct);

      break;
  }

  return 0; // never gets here
}
