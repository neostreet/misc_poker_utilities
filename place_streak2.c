#include <stdio.h>

static char usage[] = "usage: skeleton filename\n";
static char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_no;
  int dbg_line_no;
  int dbg;
  int prev_place;
  int place;
  int place_streak;
  int max_place_streak;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  dbg_line_no = -1;
  place_streak = 0;
  max_place_streak = 0;

  for ( ; ; ) {
    fscanf(fptr,"%d",&place);

    if (feof(fptr))
      break;

    line_no++;

    if (line_no == dbg_line_no)
      dbg = 1;

    if (line_no == 1)
      place_streak = 1;
    else {
      if (place == prev_place)
        place_streak++;
      else {
        printf("%d: %d\n",prev_place,place_streak);

        if (place_streak > max_place_streak)
          max_place_streak = place_streak;

        place_streak = 1;
      }
    }

    prev_place = place;
  }

  fclose(fptr);

  printf("%d: %d\n",prev_place,place_streak);
  printf("\nmax_place_streak: %d\n",max_place_streak);

  return 0;
}
