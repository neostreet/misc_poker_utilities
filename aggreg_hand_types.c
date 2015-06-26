#include <stdio.h>
#include <string.h>
#include "str_list.h"

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: aggreg_hand_types offset filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char *plain_hand_types[] = {
  "high card",
  "one pair",
  "two pair",
  "three of a kind",
  "straight",
  "flush",
  "full house",
  "four of a kind",
  "straight flush",
  "royal flush"
};
#define NUM_HAND_TYPES (sizeof plain_hand_types / sizeof (char *))
static int plain_hand_type_lens[NUM_HAND_TYPES];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_hand_type_and_delta(
  char *line,
  int line_len,
  int offset,
  char **hand_type_ptr,
  int *delta_ptr);

int main(int argc,char **argv)
{
  int n;
  int offset;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  struct info_list hand_types;
  struct info_list_elem *work_elem;
  int ix;
  char *hand_type;
  int delta;
  int totals[4];
  double work;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&offset);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  line_no = 0;
  hand_types.num_elems = 0;

  for (n = 0; n < NUM_HAND_TYPES; n++) {
    plain_hand_type_lens[n] = strlen(plain_hand_types[n]);
    add_info_list_elem(&hand_types,plain_hand_types[n],0,0,0,0,true);
  }

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    retval = get_hand_type_and_delta(line,line_len,offset,&hand_type,&delta);

    if (retval) {
      printf("get_hand_type_and_delta() failed on line %d: %d\n",line_no,retval);
      return 3;
    }

    if (member_of_info_list(&hand_types,hand_type,&ix)) {
      if (get_info_list_elem(&hand_types,ix,&work_elem)) {
        if (delta > 0) {
          work_elem->int1++;
          work_elem->int3 += delta;
        }
        else {
          work_elem->int2++;
          work_elem->int4 += delta;
        }
      }
    }
    else {
      if (delta > 0)
        add_info_list_elem(&hand_types,hand_type,1,0,delta,0,true);
      else
        add_info_list_elem(&hand_types,hand_type,0,1,0,delta,true);
    }
  }

  fclose(fptr);

  work_elem = hand_types.first_elem;

  for (n = 0; n < 4; n++)
    totals[n] = 0;

  for (n = 0; n < hand_types.num_elems; n++) {
    if (work_elem->int1 + work_elem->int2 == 0)
      work = (double)0;
    else {
      work = (double)(work_elem->int3 + work_elem->int4) /
        (double)(work_elem->int1 + work_elem->int2);
    }

    printf("%10d %10d %10d %5d %5d %5d %11.2lf   %s\n",
      work_elem->int3 + work_elem->int4,
      work_elem->int3,
      work_elem->int4,
      work_elem->int1 + work_elem->int2,
      work_elem->int1,
      work_elem->int2,
      work,
      work_elem->str);
    totals[0] += work_elem->int3;
    totals[1] += work_elem->int4;
    totals[2] += work_elem->int1;
    totals[3] += work_elem->int2;
    work_elem = work_elem->next_elem;
  }

  free_info_list(&hand_types);

  work = (double)(totals[0] + totals[1]) /
    (double)(totals[2] + totals[3]);

  printf("\n%10d %10d %10d %5d %5d %5d %11.2lf\n",
    totals[0] + totals[1],
    totals[0],totals[1],totals[2] + totals[3],totals[2],totals[3],work);

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

static int get_hand_type_and_delta(
  char *line,
  int line_len,
  int offset,
  char **hand_type_ptr,
  int *delta_ptr)
{
  int m;
  int n;
  char delta_str[11];

  for (n = NUM_HAND_TYPES - 1; (n >= 0); n--) {
    if (!strncmp(&line[offset],plain_hand_types[n],plain_hand_type_lens[n]))
      break;
  }

  if (n < 0)
    return 1;

  *hand_type_ptr = plain_hand_types[n];

  for (m = 0; m < 10; m++) {
    if (line[m] != ' ')
      break;
  }

  if (m == 10)
    return 2;

  for (n = 0; m < 10; m++)
    delta_str[n++] = line[m];

  delta_str[n] = 0;

  sscanf(delta_str,"%d",delta_ptr);

  return 0;
}
