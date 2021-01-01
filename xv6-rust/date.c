// The code for dayofweek was obtained at:
// https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week

#ifdef CS333_P1
#include "types.h"
#include "user.h"
#include "date.h"

static char *months[] = {"NULL", "Jan", "Feb", "Mar", "Apr",
  "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char *days[] = {"Sun", "Mon", "Tue", "Wed",
  "Thu", "Fri", "Sat"};

static int
dayofweek(int y, int m, int d)
{
  return (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7;
}

int
main(int argc, char *argv[])
{
  int day;
  struct rtcdate r;

  if (date(&r)) {
    printf(2,"Error: date call failed. %s at line %d\n",
        __FILE__, __LINE__);
    exit();
  }

  day = dayofweek(r.year, r.month, r.day);

  printf(1, "%s %s %d", days[day], months[r.month], r.day);
  printf(1, " ");
  if (r.hour < 10) printf(1, "0");
  printf(1, "%d:", r.hour);
  if (r.minute < 10) printf(1, "0");
  printf(1, "%d:", r.minute);
  if (r.second < 10) printf(1, "0");
  printf(1, "%d UTC %d\n", r.second, r.year);

  exit();
}
#endif
