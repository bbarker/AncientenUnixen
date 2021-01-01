#ifdef CS333_P5
// this is an ugly series of if statements but it works
void
print_mode(struct stat* st)
{
  switch (st->type) {
    case T_DIR: printf(1, "d"); break;
    case T_FILE: printf(1, "-"); break;
    case T_DEV: printf(1, "c"); break;
    default: printf(1, "?");
  }

  if (st->mode.flags.u_r)
    printf(1, "r");
  else
    printf(1, "-");

  if (st->mode.flags.u_w)
    printf(1, "w");
  else
    printf(1, "-");

  if ((st->mode.flags.u_x) & (st->mode.flags.setuid))
    printf(1, "S");
  else if (st->mode.flags.u_x)
    printf(1, "x");
  else
    printf(1, "-");

  if (st->mode.flags.g_r)
    printf(1, "r");
  else
    printf(1, "-");

  if (st->mode.flags.g_w)
    printf(1, "w");
  else
    printf(1, "-");

  if (st->mode.flags.g_x)
    printf(1, "x");
  else
    printf(1, "-");

  if (st->mode.flags.o_r)
    printf(1, "r");
  else
    printf(1, "-");

  if (st->mode.flags.o_w)
    printf(1, "w");
  else
    printf(1, "-");

  if (st->mode.flags.o_x)
    printf(1, "x");
  else
    printf(1, "-");

  return;
}
#endif
