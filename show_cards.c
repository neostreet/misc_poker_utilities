#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

static char usage[] = "show_cards (-missing)\n";

static char *denoms[] = {
 "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"
};
#define NUM_DENOMS (sizeof denoms / sizeof (char *))
static char *suits[] = {
 "c", "d", "h", "s"
};
#define NUM_SUITS (sizeof suits / sizeof (char *))

#define BUF_LEN 80
static char buf[BUF_LEN];

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bMissing;
  struct stat statbuf;

  if (argc > 2) {
    printf(usage);
    return 1;
  }

  bMissing = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-missing"))
      bMissing = true;
    else
      break;
  }

  if (curr_arg != argc) {
    printf(usage);
    return 2;
  }

  for (m = 0; m < NUM_SUITS; m++) {
    for (n = 0; n < NUM_DENOMS; n++) {
      sprintf(buf,"%s%s",denoms[n],suits[m]);

      if (!bMissing) {
        if (!stat(buf,&statbuf))
          printf("%s",buf);
        else
          printf("  ");
      }
      else {
        if (stat(buf,&statbuf))
          printf("%s",buf);
        else
          printf("  ");
      }

      if (n < NUM_DENOMS - 1)
        putchar(' ');
      else
        putchar(0x0a);
    }
  }

  return 0;
}
