#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

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
  struct stat statbuf;

  for (m = 0; m < NUM_SUITS; m++) {
    for (n = 0; n < NUM_DENOMS; n++) {
      sprintf(buf,"%s%s",denoms[n],suits[m]);

      if (!stat(buf,&statbuf))
        printf("%s",buf);
      else
        printf("  ");

      if (n < NUM_DENOMS - 1)
        putchar(' ');
      else
        putchar(0x0a);
    }
  }

  return 0;
}
