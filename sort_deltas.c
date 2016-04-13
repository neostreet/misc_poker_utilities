#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <string.h>

#define MAX_DELTA_STR_LEN 10

#define TAB 0x09

static char usage[] =
"usage: sort_deltas (-no_sort) (-reverse) (-offsetoffset) (-float)\n"
"  (-line_numbers) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static bool bReverse;
static bool bFloat;

static char **cppt;

static int *delta;
static double *doubles;

static char malloc_fail1[] = "malloc of %d chars failed\n";
static char malloc_fail2[] = "malloc of %d char pointers failed\n";
static char malloc_fail3[] = "malloc of %d ints failed\n";
static char malloc_fail4[] = "malloc of %d doubles failed\n";

int compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bNoSort;
  int offset;
  bool bLineNumbers;
  struct stat statbuf;
  int mem_amount;
  char *mempt;
  FILE *fptr;
  int num_lines;
  int file_ix;
  int *ixs;
  int cppt_ix;
  int chara;
  char delta_buf[MAX_DELTA_STR_LEN+1];

  if ((argc < 2) || (argc > 7)) {
    printf(usage);
    return 1;
  }

  bNoSort = false;
  bReverse = false;
  bFloat = false;
  offset = 0;
  bLineNumbers = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-no_sort"))
      bNoSort = true;
    else if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = true;
    else if (!strcmp(argv[curr_arg],"-float"))
      bFloat = true;
    else if (!strncmp(argv[curr_arg],"-offset",7))
      sscanf(&argv[curr_arg][7],"%d",&offset);
    else if (!strcmp(argv[curr_arg],"-line_numbers"))
      bLineNumbers = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (stat(argv[curr_arg],&statbuf) == -1) {
    printf("couldn't get status of %s\n",argv[curr_arg]);
    return 3;
  }
  else
    mem_amount = (size_t)statbuf.st_size;

  if ((mempt = (char *)malloc(mem_amount)) == NULL) {
    printf(malloc_fail1,mem_amount);
    return 4;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    free(mempt);
    return 5;
  }

  num_lines = 0;

  for (n = 0; ; n++) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == '\n') {
      num_lines++;
      chara = 0;
    }

    mempt[n] = chara;
  }

  fclose(fptr);

  if ((cppt = (char **)malloc(num_lines * sizeof (char *))) == NULL) {
    printf(malloc_fail2,num_lines);
    free(mempt);
    return 6;
  }

  if (!bFloat) {
    if ((delta = (int *)malloc(num_lines * sizeof (int))) == NULL) {
      printf(malloc_fail3,num_lines);
      free(cppt);
      free(mempt);
      return 7;
    }
  }
  else {
    if ((doubles = (double *)malloc(num_lines * sizeof (double))) == NULL) {
      printf(malloc_fail4,num_lines);
      free(cppt);
      free(mempt);
      return 7;
    }
  }

  if ((ixs = (int *)malloc(num_lines * sizeof (int))) == NULL) {
    printf(malloc_fail3,num_lines);

    if (!bFloat)
      free(delta);
    else
      free(doubles);

    free(cppt);
    free(mempt);
    return 8;
  }

  for (n = 0; n < num_lines; n++)
    ixs[n] = n;

  file_ix = 0;
  cppt_ix = 0;

  for (n = 0; n < mem_amount; n++) {
    if (!mempt[n]) {
      cppt[file_ix] = &mempt[cppt_ix];
      cppt_ix = n + 1;

      for (p = 0; ; p++) {
        chara = cppt[file_ix][offset+p];

        if (chara != ' ')
          break;
      }

      for (m = 0; m < MAX_DELTA_STR_LEN; m++) {
        chara = cppt[file_ix][offset+p+m];

        if ((chara == ' ') || (chara == TAB))
          break;

        delta_buf[m] = chara;
      }

      delta_buf[m] = 0;

      if (!bFloat)
        sscanf(delta_buf,"%d",&delta[file_ix]);
      else
        sscanf(delta_buf,"%lf",&doubles[file_ix]);

      file_ix++;
    }
  }

  if (!bNoSort)
    qsort(ixs,num_lines,sizeof (int),compare);

  for (n = 0; n < num_lines; n++) {
    if (!bLineNumbers)
      printf("%s\n",cppt[ixs[n]]);
    else
      printf("%s (%d)\n",cppt[ixs[n]],ixs[n] + 1);
  }

  free(ixs);

  if (!bFloat)
    free(delta);
  else
    free(doubles);

  free(cppt);
  free(mempt);

  return 0;
}

int compare(const void *elem1,const void *elem2)
{
  int int1;
  int int2;

  int1 = *(int *)elem1;
  int2 = *(int *)elem2;

  if (!bFloat) {
    if (delta[int1] == delta[int2])
      return strcmp(cppt[int2],cppt[int1]);

    if (!bReverse)
      return delta[int1] - delta[int2];
    else
      return delta[int2] - delta[int1];
  }
  else {
    if (doubles[int1] == doubles[int2])
      return strcmp(cppt[int2],cppt[int1]);

    if (!bReverse) {
      if (doubles[int1] < doubles[int2])
        return -1;
      else
        return 1;
    }
    else {
      if (doubles[int2] < doubles[int1])
        return -1;
      else
        return 1;
    }
  }
}
