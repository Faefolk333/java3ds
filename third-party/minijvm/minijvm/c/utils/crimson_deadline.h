#ifndef CRIMSON_DEADLINE_H
#define CRIMSON_DEADLINE_H
#include <time.h>
/* Add a nonnegative millisecond duration to an absolute, normalized deadline. */
static inline void crimson_add_millis(struct timespec *t,long long ms){
    t->tv_sec += ms / 1000;
    t->tv_nsec += (ms % 1000) * 1000000;
    t->tv_sec += t->tv_nsec / 1000000000;
    t->tv_nsec %= 1000000000;
}
#endif
