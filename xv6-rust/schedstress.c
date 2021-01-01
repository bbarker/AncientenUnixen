#ifdef CS333_P3P4
#include "types.h"
#include "user.h"

// Noah Zentzis, 2016

#define MAX_PRIORITY 7

// stress-test the MLFQ scheduler
// 
// Note that this program doesn't necessarily do proper error-checking for all
// system calls, since it's not meant to test that. This is only intended to
// verify that MLFQ is behaving properly, and adding error-handling would make
// it more complicated than it really needs to be.

// wait for all children - just keep calling wait() until it fails due to not
// having any children.
void
waitall(void) {
  while(wait() != -1);
}

// high-priority locking process. starts a process that locks itself into queue
// Q until it counts up to N.
void
queuebound(int q, unsigned int n) {
  if(fork() == 0) {
    int pid = getpid();
    // busywait
    for(unsigned int i = 0;i < n;i++) {
      setpriority(pid, q);
    }
    exit();
  }
}

// Test 1: queue priority. Start a few low-priority processes that print values
// on a delay, then start four CPU-bound processes that lock themselves into
// priority 0. Values should stop printing while the high-priority processes
// run.
void
test1(void) {
  printf(1, "\n> starting test 1\n");

  // start low-priority printing processes
  for(int i = 0;i < 2;i++) {
    if(fork() == 0) {
      setpriority(getpid(), MAX_PRIORITY); // deprioritize ourselves

      // lock ourselves into low priority
      for(int n = 0;n < 5;n++) {
        printf(1, "low-priority #%d\n", n);
        setpriority(getpid(), MAX_PRIORITY);
        sleep(10);
      }
      exit();
    }
  }
  sleep(20); // wait for a few low priority values to print

  // start high-priority processes
  for(int i = 0;i < 4;i++) {
    if(fork() == 0) {
      int pid = getpid();
      printf(1, "high-priority start\n");

      // busywait
      for(unsigned int i = 0;i < 0x100000;i++) {
        setpriority(pid, 0); // lock into queue 0
      }

      printf(1, "high-priority end\n");
      exit();
    }
  }

  waitall();
  printf(1, "\n> test 1 complete\n");
}

// Test 2: promotion. This makes it easy to check that promotion into queue 0
// functions as expected.
//
// This test functions by creating one processes for each priority level that
// waits for a short period of time before printing a value and exiting. Then,
// before the wait expires, it creates several high-priority processes that lock
// themselves into the highest-priority queue.
//
// The highest-priority process should run quickly and print its value. Each
// subsequent promotion delay, a new process will be moved into queue 0 and get
// a chance to run. This will manifest as an ascending sequence of numbers (one
// for each queue).
void
test2(void) {
  printf(1, "\n> starting test 2\n");

  // start processes for each queue
  for(int i = 0;i <= MAX_PRIORITY;i++) {
    int p = fork();
    if(p == 0) {
      sleep(10);
      printf(1, "queue %d!\n", i);
      exit();
    } else {
      setpriority(p, i);
    }
  }

  // high-priority processes
  for(int i = 0;i < 8;i++)
    queuebound(0, 0x80000);

  // wait for children
  waitall();
  printf(1, "\n> test 2 complete\n");
}

// Test 3: queue append.
//
// Verifies that queue 1 is appended to queue 0 during promotion. This test
// starts 4 busyloops in queue 0 and 4 in a lower-priority queue. Then, you can
// use C-L to show the different queues. Once promotion occurs, the
// lower-priority loops should be visible in queue 0 in the same order as they
// were in the lower-priority queue.
void
test3(void) {
  printf(1, "\n> starting test 3\n");
  if(MAX_PRIORITY < 2) {
    printf(2, "! test 3 requires 3 queues!\n");
    return;
  }

  // start low-priority loops
  for(int i = 0;i < 4;i++) {
    int p = fork();
    if(p == 0) {
      p = getpid();
      // wait, so the other loops can start
      sleep(10);

      // lock to priority 0
      for(int j = 0;j < 0x100000;j++)
        setpriority(p, 0);
      exit();
    } else {
      setpriority(p, 2);
    }
  }

  // start high-priority loops
  for(int i = 0;i < 4;i++)
    queuebound(0, 0x800000);

  // wait for children
  waitall();
  printf(1, "\n> test 3 complete\n");
}

// Test 3: demotion. This makes it easy to check that demotion into queue 0
// functions as expected.
//
// This test functions by creating one processes for each priority level that
// waits for a short period of time before printing a value and exiting. Then,
// before the wait expires, it creates several high-priority busyloops.
//
// The highest-priority process should run quickly and print its value. Each
// time the CPU-bound processes are demoted, a new process will be able to run
// and will display its value. This will manifest as an ascending sequence of
// numbers (one for each queue).
void
test4(void) {
  printf(1, "\n> starting test 4\n");

  // start processes for each queue
  for(int i = 0;i <= MAX_PRIORITY;i++) {
    int p = fork();
    if(p == 0) {
      sleep(10);
      printf(1, "queue %d!\n", i);
      exit();
    } else {
      setpriority(p, i);
    }
  }

  // high-priority processes
  for(int i = 0;i < 8;i++) {
    if(fork() == 0) {
      int p = getpid();
      // let the others get established so we all move down at once
      for(int j = 0;j < 0x100000;j++)
        setpriority(p, 0);

      for(int j = 0;j < 0x800000;j++);
      exit();
    }
  }

  // wait for children
  waitall();
  printf(1, "\n> test 4 complete\n");
}

int
main(int argc, char **argv) {
  int test = 0;
  if(argc == 2) test = atoi(argv[1]);

  if(test == 0) printf(1, "stressing scheduler...\n");
  if(test == 1 || test == 0) test1();
  if(test == 2 || test == 0) test2();
  if(test == 3 || test == 0) test3();
  if(test == 4 || test == 0) test4();
  exit();
}
#endif
