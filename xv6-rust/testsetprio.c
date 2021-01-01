#ifdef CS333_P3P4
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid, prio, rc;

  if (argc == 1) {
    pid = getpid();
    prio = 0;
  }
  else {
    if (argc != 3) {
      printf(2, "Error: invalid pid or priority\n");
      printf(2, "Usage: %s [<pid> <prio>]\n",argv[0]);
      exit();
    }
    else {
      pid = atoi(argv[1]);
      prio = atoi(argv[2]);
    }
  }

  rc = setpriority(pid, prio);
  if (rc != 0) {
      printf(2, "Error: invalid pid\n");
      printf(2, "Usage: %s [<pid> <prio>]\n",argv[0]);
  }
  exit();
}
#endif
