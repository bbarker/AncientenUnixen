#ifdef CS333_P5
#define TRUE 1
#define FALSE 0
#define PASS 1
#define NOPASS 0

// this routine requires that the caller define "int rc".
#define doTest(A, B) {                          \
  rc = (A)((B));                                \
  if (rc == NOPASS) {                           \
    printf(2, "Error! %s failed\n", #A);        \
    exit();                                     \
  }                                             \
}

// this routine requires that the caller define "int rc".
// check() doesn't handle return codes correctly.
#define check(A) {                              \
  rc = (A);                                     \
  if (rc) {                                     \
    printf(2,"Error: %s failed\n", #A);         \
    return FALSE;                               \
  }                                             \
}

// used in several places
int NUMPERMSTOCHECK = 4;
int perms[] = {01544, 01454, 01445, 01666};
enum {procuid, procgid, fileuid, filegid};
int testperms[][4] = {
  // {proc-uid, proc-gid, file-uid, file-gid}
  {212, 323, 212, 434}, // uids match
  {212, 323, 434, 323}, // gids match
  {111, 222, 333, 444}, // match on other
  {111, 222, 111, 222},
};
#endif
