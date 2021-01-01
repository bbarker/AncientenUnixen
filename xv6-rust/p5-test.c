// A grand unified test for CS333 project 5.
// P5 adds protection (via UNIX-like mode bits) to xv6. This
// program is provided for testing.
#ifdef CS333_P5

#include "types.h"
#include "user.h"
#include "stat.h"
#include "p5-test.h"

static int
canRun(char *name)
{
  int rc, uid, gid;
  struct stat st;

  uid = getuid();
  gid = getgid();
  check(stat(name, &st));
  if (uid == st.uid) {
    if (st.mode.flags.u_x)
      return TRUE;
    else {
      printf(2, "UID match. Execute permission for user not set.\n");
      return FALSE;
    }
  }
  if (gid == st.gid) {
    if (st.mode.flags.g_x)
      return TRUE;
    else {
      printf(2, "GID match. Execute permission for group not set.\n");
      return FALSE;
    }
  }
  if (st.mode.flags.o_x) {
    return TRUE;
  }

  printf(2, "Execute permission for other not set.\n");
  return FALSE;  // failure. Can't run
}

static int
doSetuidTest (char **cmd)
{
  int rc, i;
  char *test[] = {"UID match", "GID match", "Other", "Should Fail"};

  printf(1, "\nTesting the set uid bit.\n\n");

  for (i=0; i<NUMPERMSTOCHECK; i++) {
    printf(1, "Starting test: %s.\n", test[i]);
    check(setuid(testperms[i][procuid]));
    check(setgid(testperms[i][procgid]));
    printf(1, "Process uid: %d, gid: %d\n", getuid(), getgid());
    check(chown(cmd[0], testperms[i][fileuid]));
    check(chgrp(cmd[0], testperms[i][filegid]));
    printf(1, "File uid: %d, gid: %d\n",
        testperms[i][fileuid], testperms[i][filegid]);
    check(chmod(cmd[0], perms[i]));
    printf(1, "perms set to %d for %s\n", perms[i], cmd[0]);

    rc = fork();
    if (rc < 0) {    // fork failed
      printf(2, "The fork() system call failed. That's pretty catastrophic. Ending test\n");
      return NOPASS;
    }
    if (rc == 0) {   // child
      exec(cmd[0], cmd);
      if (i != NUMPERMSTOCHECK-1) printf(2, "**** exec call for %s **FAILED**.\n",  cmd[0]);
      else printf(2, "**** exec call for %s **FAILED as expected.\n", cmd[0]);
      exit();
    }
    wait();
  }
  chmod(cmd[0], 00755);  // total hack but necessary. sigh
  printf(1, "Test Passed\n");
  return PASS;
}

static int
doUidTest (char **cmd)
{
  int i, rc, uid, startuid, testuid, baduidcount = 3;
  int baduids[] = {32767+5, -41, ~0};  // 32767 is max value

  printf(1, "\nExecuting setuid() test.\n\n");

  startuid = uid = getuid();
  testuid = ++uid;
  rc = setuid(testuid);
  if (rc) {
    printf(2, "setuid system call reports an error.\n");
    return NOPASS;
  }
  uid = getuid();
  if (uid != testuid) {
    printf(2, "ERROR! setuid claims to have worked but really didn't!\n");
    printf(2, "uid should be %d but is instead %d\n", testuid, uid);
    return NOPASS;
  }
  for (i=0; i<baduidcount; i++) {
    rc = setuid(baduids[i]);
    if (rc == 0) {
      printf(2, "Tried to set the uid to a bad value (%d) and setuid()failed to fail. rc == %d\n",
                      baduids[i], rc);
      return NOPASS;
    }
    rc = getuid();
    if (baduids[i] == rc) {
      printf(2, "ERROR! Gave setuid() a bad value (%d) and it failed to fail. gid: %d\n",
        baduids[i],rc);
      printf(2, "Valid UIDs are in the range [0, 32767] only!\n");
      return NOPASS;
    }
  }
  setuid(startuid);
  printf(1, "Test Passed\n");
  return PASS;
}

static int
doGidTest (char **cmd)
{
  int i, rc, gid, startgid, testgid, badgidcount = 3;
  int badgids[] = {32767+5, -41, ~0};  // 32767 is max value

  printf(1, "\nExecuting setgid() test.\n\n");

  startgid = gid = getgid();
  testgid = ++gid;
  rc = setgid(testgid);
  if (rc) {
    printf(2, "setgid system call reports an error.\n");
    return NOPASS;
  }
  gid = getgid();
  if (gid != testgid) {
    printf(2, "ERROR! setgid claims to have worked but really didn't!\n");
    printf(2, "gid should be %d but is instead %d\n", testgid, gid);
    return NOPASS;
  }
  for (i=0; i<badgidcount; i++) {
    rc = setgid(badgids[i]);
    if (rc == 0) {
      printf(2, "Tried to set the gid to a bad value (%d) and setgid()failed to fail. rc == %d\n",
        badgids[i], rc);
      return NOPASS;
    }
    rc = getgid();
    if (badgids[i] == rc) {
      printf(2, "ERROR! Gave setgid() a bad value (%d) and it failed to fail. gid: %d\n",
        badgids[i], rc);
      printf(2, "Valid GIDs are in the range [0, 32767] only!\n");
      return NOPASS;
    }
  }
  setgid(startgid);
  printf(1, "Test Passed\n");
  return PASS;
}

static int
doChmodTest(char **cmd)
{
  int i, rc, mode, testmode;
  struct stat st;

  printf(1, "\nExecuting chmod() test.\n\n");

  check(stat(cmd[0], &st));
  mode = st.mode.asInt;

  for (i=0; i<NUMPERMSTOCHECK; i++) {
    check(chmod(cmd[0], perms[i]));
    check(stat(cmd[0], &st));
    testmode = st.mode.asInt;
    if (mode == testmode) {
      printf(2, "Error! Unable to test.\n");
      printf(2, "\tfile mode (%d) == testmode (%d) for file (%s) in test %d\n",
        mode, testmode, cmd[0], i);
      return NOPASS;
    }
  }
  chmod(cmd[0], 00755); // hack
  printf(1, "Test Passed\n");
  return PASS;
}

static int
doChownTest(char **cmd)
{
  int rc, uid1, uid2;
  struct stat st;

  printf(1, "\nExecuting chown test.\n\n");

  stat(cmd[0], &st);
  uid1 = st.uid;

  rc = chown(cmd[0], uid1+1);
  if (rc != 0) {
    printf(2, "Error! chown() failed on setting new owner. %d as rc.\n", rc);
    return NOPASS;
  }

  stat(cmd[0], &st);
  uid2 = st.uid;

  if (uid1 == uid2) {
    printf(2, "Error! test failed. Old uid: %d, new uid: uid2, should differ\n",
        uid1, uid2);
    return NOPASS;
  }
  chown(cmd[0], uid1);  // put back the original
  printf(1, "Test Passed\n");
  return PASS;
}

static int
doChgrpTest(char **cmd)
{
  int rc, gid1, gid2;
  struct stat st;

  printf(1, "\nExecuting chgrp test.\n\n");

  stat(cmd[0], &st);
  gid1 = st.gid;

  rc = chgrp(cmd[0], gid1+1);
  if (rc != 0) {
    printf(2, "Error! chgrp() failed on setting new group.\n");
    return NOPASS;
  }

  stat(cmd[0], &st);
  gid2 = st.gid;

  if (gid1 == gid2) {
    printf(2, "Error! test failed. Old gid: %d, new gid: gid2, should differ\n",
                    gid1, gid2);
    return NOPASS;
  }
  chgrp(cmd[0], gid1);  // put back the original
  printf(1, "Test Passed\n");
  return PASS;
}

static int
doExecTest(char **cmd)
{
  int i, rc, uid, gid;
  struct stat st;

  printf(1, "\nExecuting exec test.\n\n");

  if (!canRun(cmd[0])) {
    printf(2, "Unable to run %s. test aborted.\n", cmd[0]);
    return NOPASS;
  }

  check(stat(cmd[0], &st));
  uid = st.uid;
  gid = st.gid;

  for (i=0; i<NUMPERMSTOCHECK; i++) {
    check(setuid(testperms[i][procuid]));
    check(setgid(testperms[i][procgid]));
    check(chown(cmd[0], testperms[i][fileuid]));
    check(chgrp(cmd[0], testperms[i][filegid]));
    check(chmod(cmd[0], perms[i]));
    if (i != NUMPERMSTOCHECK-1)
      printf(2, "The following test should not produce an error.\n");
    else
      printf(2, "The following test should fail.\n");
    rc = fork();
    if (rc < 0) {    // fork failed
      printf(2, "The fork() system call failed. That's pretty catastrophic. Ending test\n");
      return NOPASS;
    }
    if (rc == 0) {   // child
      exec(cmd[0], cmd);
      if (i != NUMPERMSTOCHECK-1) printf(2, "**** exec call for %s **FAILED**.\n",  cmd[0]);
      else printf(2, "**** exec call for %s **FAILED as expected.\n", cmd[0]);
      exit();
    }
    wait();
  }
  chown(cmd[0], uid);
  chgrp(cmd[0], gid);
  chmod(cmd[0], 00755);
  printf(1, "Requires user visually confirms PASS/FAIL\n");
  return PASS;
}

void
printMenu(void)
{
  int i = 0;
  printf(1, "\n");
  printf(1, "%d. exit program\n", i++);
  printf(1, "%d. Proc UID\n", i++);
  printf(1, "%d. Proc GID\n", i++);
  printf(1, "%d. chmod()\n", i++);
  printf(1, "%d. chown()\n", i++);
  printf(1, "%d. chgrp()\n", i++);
  printf(1, "%d. exec()\n", i++);
  printf(1, "%d. setuid\n", i++);
}

int
main(int argc, char *argv[])
{
  int rc, select, done;
  char buf[5];

  // test strings
  char *t0[] = {'\0'}; // dummy
  char *t1[] = {"testsetuid", '\0'};

  while (1) {
    done = FALSE;
    printMenu();
    printf(1, "Enter test number: ");
    gets(buf, 5);
    if ((buf[0] == '\n') || (buf[0] == '\0')) continue;
    select = atoi(buf);
    switch (select) {
      case 0: done = TRUE; break;
      case 1: doTest(doUidTest,    t0); break;
      case 2: doTest(doGidTest,    t0); break;
      case 3: doTest(doChmodTest,  t1); break;
      case 4: doTest(doChownTest,  t1); break;
      case 5: doTest(doChgrpTest,  t1); break;
      case 6: doTest(doExecTest,   t1); break;
      case 7: doTest(doSetuidTest, t1); break;
      default:
        printf(1, "Error:invalid test number.\n");
    }

    if (done) break;
  }

  printf(1, "\nDone for now\n");
  free(buf);
  exit();
}

#endif
