// Required to include clock_gettime
#include <time.h>

// Emulate clock_nanosleep() on MacOS
// https://fossies.org/linux/privat/Time-HiRes-1.9760.tar.gz/Time-HiRes-1.9760/HiRes.xs?m=t
// https://github.com/jj1bdx/dump1090/blob/ba0b63ee1eab28e42c61d9005eadd036503d2bd7/sdr_ifile.c
#if defined(__APPLE__)
#include <mach/mach_time.h>
#define IV_1E9 1000000000
#define TIMER_ABSTIME 0x01
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp);
#endif