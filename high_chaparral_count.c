#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

#define MAX_DELTA_STR_LEN 10

#define TAB 0x09

static char usage[] =
"usage: high_chaparral_count (-verbose) (-total) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char **cppt;

static int *delta;

static char malloc_fail1[] = "malloc of %d chars failed\n";
static char malloc_fail2[] = "malloc of %d char pointers failed\n";
static char malloc_fail3[] = "malloc of %d ints failed\n";

int compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bVerbose;
  bool bTotal;
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
  int high_chaparral_total;
  int work;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bTotal = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-total"))
      bTotal = true;
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

  if ((delta = (int *)malloc(num_lines * sizeof (int))) == NULL) {
    printf(malloc_fail3,num_lines);
    free(cppt);
    free(mempt);
    return 7;
  }

  if ((ixs = (int *)malloc(num_lines * sizeof (int))) == NULL) {
    printf(malloc_fail3,num_lines);

    free(delta);

    free(cppt);
    free(mempt);
    return 9;
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
        chara = cppt[file_ix][p];

        if (chara != ' ')
          break;
      }

      for (m = 0; m < MAX_DELTA_STR_LEN; m++) {
        chara = cppt[file_ix][p+m];

        if ((chara == ' ') || (chara == TAB))
          break;

        delta_buf[m] = chara;
      }

      delta_buf[m] = 0;

      sscanf(delta_buf,"%d",&delta[file_ix]);

      file_ix++;
    }
  }

  qsort(ixs,num_lines,sizeof (int),compare);

  if (bTotal)
    high_chaparral_total = 0;

  for (n = 0; n < num_lines; n++) {
    if (cppt[ixs[n]][0] == '-')
      break;

    if (bTotal) {
      sscanf(&cppt[ixs[n]][0],"%d",&work);
      high_chaparral_total += work;
    }
  }

  if (!bVerbose) {
    if (!bTotal)
      printf("%d\n",n);
    else
      printf("%d\n",high_chaparral_total);
  }
  else {
    if (!bTotal)
      printf("%d (%d) %s\n",n,num_lines,save_dir);
    else
      printf("%d (%d) %s\n",high_chaparral_total,num_lines,save_dir);
  }

  free(ixs);

  free(delta);

  free(cppt);
  free(mempt);

  return 0;
}

int compare(const void *elem1,const void *elem2)
{
  int int1;
  int int2;
  int delta1;
  int delta2;

  int1 = *(int *)elem1;
  int2 = *(int *)elem2;

  delta1 = delta[int1];
  delta2 = delta[int2];

  if (delta1 < 0)
    delta1 *= -1;

  if (delta2 < 0)
    delta2 *= -1;

  if (delta1 == delta2)
    return strcmp(cppt[int2],cppt[int1]);

  return delta2 - delta1;
}
