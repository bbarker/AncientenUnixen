// halt the system.
#include "types.h"
#include "user.h"

int
main(void) {
  printf(1, "Shutting down...\n");
  halt();
  exit();
}
