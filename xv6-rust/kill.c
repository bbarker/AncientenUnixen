#include "types.h"
#include "stat.h"
#include "user.h"
#ifdef PDX_XV6
#include "pdx.h"
#endif // PDX_XV6

int
main(int argc, char **argv)
{
  int i;

  if(argc < 2){
    printf(2, "usage: kill pid...\n");
    exit();
  }
  for(i=1; i<argc; i++)
    kill(atoi(argv[i]));
  exit();
}
