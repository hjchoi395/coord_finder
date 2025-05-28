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

#define DC_DELAY_MS 100  // data center 왕복 지연 시간 (밀리초)

// 고해상도 시간 측정 함수 (밀리초 단위 반환)
static double current_time_ms() {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    // 나노초 단위 계산 후 밀리초로 변환
    return (double)counter.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
    struct timespec ts;
    // MONOTONIC은 시스템 시간이 변경되어도 계속 증가
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
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

        double t0 = current_time_ms();

        if (cache_lookup(input, value)) {
            double t1 = current_time_ms();
            printf("CACHE HIT : %s -> %s (%.3f ms)\n",
                   input, value, t1 - t0);
        } else {
#ifdef _WIN32
            Sleep(DC_DELAY_MS);
#else
            usleep(DC_DELAY_MS * 1000);
#endif
            if (db_lookup(input, value)) {
                cache_insert(input, value);
                double t1 = current_time_ms();
                printf("CACHE MISS: loaded from DB -> %s (%.3f ms including ~%d ms DC delay)\n",
                       value, t1 - t0, DC_DELAY_MS);
            } else {
                double t1 = current_time_ms();
                printf("NOT FOUND  : %s (%.3f ms)\n",
                       input, t1 - t0);
            }
        }
    }

    save_cache();
    print_cache();

    return 0;
}