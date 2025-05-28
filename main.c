#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <unistd.h>
#endif

#include "cache.h"
#include "db.h"

// 고해상도 시간 측정 함수 (마이크로초 단위 반환)
static double current_time_us() {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1e6 / (double)freq.QuadPart;
#else
    struct timespec ts;
    // MONOTONIC은 시스템 시간이 변경되어도 계속 증가
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
#endif
}

int main() {
    char input[64], value[128];

    init_cache();
    load_cache();  // 이전 실행 시 저장된 캐시 로드

    while (1) {
        printf(">> Enter base name (or 'exit'): ");
        if (scanf("%63s", input) != 1) break;
        if (strcmp(input, "exit") == 0) break;

        double t0 = current_time_us();

        if (cache_lookup(input, value)) {
            double t1 = current_time_us();
            printf("CACHE HIT : %s -> %s (%.3f µs)\n",
                   input, value, t1 - t0);
        } else {
            if (db_lookup(input, value)) {
                cache_insert(input, value);
                double t1 = current_time_us();
                printf("CACHE MISS: loaded from DB -> %s (%.3f µs)\n",
                       value, t1 - t0);
            } else {
                double t1 = current_time_us();
                printf("NOT FOUND  : %s (%.3f µs)\n",
                       input, t1 - t0);
            }
        }
    }

    save_cache();
    print_cache();

    return 0;
}