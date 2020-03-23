#include <clock.h>

// Emulate clock_nanosleep() on MacOS
#if defined(__APPLE__)

int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp) {
    struct timespec timespec_init;
    clock_gettime(clock_id, &timespec_init);
    switch (clock_id) {
        case CLOCK_REALTIME:
        case CLOCK_MONOTONIC:
        {
            uint64_t nanos = rqtp->tv_sec * IV_1E9 + rqtp->tv_nsec;
            int success;
            if ((flags & TIMER_ABSTIME)) {
                uint64_t back = timespec_init.tv_sec * IV_1E9 + timespec_init.tv_nsec;
                nanos = nanos > back ? nanos - back : 0;
            }
            success = mach_wait_until(mach_absolute_time() + nanos) == KERN_SUCCESS;
            /* In the relative sleep, the rmtp should be filled in with
             * the 'unused' part of the rqtp in case the sleep gets
             * interrupted by a signal.  But it is unknown how signals
             * interact with mach_wait_until().  In the absolute sleep,
             * the rmtp should stay untouched. */
            if (rmtp) {
                rmtp->tv_sec  = 0;
                rmtp->tv_nsec = 0;
            }
            return success;
        }
        default:
            return -1;
    }
}

#endif