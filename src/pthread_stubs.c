#ifdef __EMSCRIPTEN__
#include <pthread.h>
#include <sched.h>

// Stub pthread_setschedparam for Emscripten
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param) {
    return 0;
}
#endif
