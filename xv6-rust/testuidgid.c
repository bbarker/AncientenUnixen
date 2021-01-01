// Example for testing part of CS333 P2.
// This is by NO MEANS a complete test.
#ifdef CS333_P2
#include "types.h"
#include "user.h"

static void
uidTest(uint nval)
{
  uint uid = getuid();
  printf(1, "Current UID is: %d\n", uid);
  printf(1, "Setting UID to %d\n", nval);
  if (setuid(nval) < 0)
    printf(2, "Error. Invalid UID: %d\n", nval);
  uid = getuid();
  printf(1, "Current UID is: %d\n", uid);
  sleep(5 * TPS);  // now type control-p
}

static void
gidTest(uint nval)
{
  uint gid = getgid();
  printf(1, "Current GID is: %d\n", gid);
  printf(1, "Setting GID to %d\n", nval);
  if (setgid(nval) < 0)
    printf(2, "Error. Invalid GID: %d\n", nval);
  setgid(nval);
  gid = getgid();
  printf(1, "Current GID is: %d\n", gid);
  sleep(5 * TPS);  // now type control-p
}

static void
forkTest(uint nval)
{
  uint uid, gid;
  int pid;

  printf(1, "Setting UID to %d and GID to %d before fork(). Value"
                  " should be inherited\n", nval, nval);

  if (setuid(nval) < 0)
    printf(2, "Error. Invalid UID: %d\n", nval);
  if (setgid(nval) < 0)
    printf(2, "Error. Invalid UID: %d\n", nval);

  printf(1, "Before fork(), UID = %d, GID = %d\n", getuid(), getgid());
  pid = fork();
  if (pid == 0) {  // child
    uid = getuid();
    gid = getgid();
    printf(1, "Child: UID is: %d, GID is: %d\n", uid, gid);
    sleep(5 * TPS);  // now type control-p
    exit();
  }
  else
    sleep(10 * TPS); // wait for child to exit before proceeding

}

static void
invalidTest(uint nval)
{
  printf(1, "Setting UID to %d. This test should FAIL\n", nval);
  if (setuid(nval) < 0)
    printf(1, "SUCCESS! The setuid sytem call indicated failure\n");
  else
    printf(2, "FAILURE! The setuid system call indicates success\n");

  printf(1, "Setting GID to %d. This test should FAIL\n", nval);
  if (setgid(nval) < 0)
    printf(1, "SUCCESS! The setgid sytem call indicated failure\n");
  else
    printf(2, "FAILURE! The setgid system call indicates success\n");

  printf(1, "Setting UID to %d. This test should FAIL\n", -1);
  if (setuid(-1) < 0)
    printf(1, "SUCCESS! The setuid sytem call indicated failure\n");
  else
    printf(2, "FAILURE! The setgid system call indicates success\n");
}

static int
testuidgid(void)
{
  uint nval, ppid;

  // get/set uid test
  nval = 100;
  uidTest(nval);

  // get/set gid test
  nval = 200;
  gidTest(nval);

  // getppid test
  ppid = getppid();
  printf(1, "My parent process is: %d\n", ppid);

  // fork tests to demonstrate UID/GID inheritance
  nval = 111;
  forkTest(nval);

  // tests for invalid values for uid and gid
  nval = 32800;   // 32767 is max value
  invalidTest(nval);

  printf(1, "Done!\n");
  return 0;
}

int
main() {
  testuidgid();
  exit();
}
#endif
