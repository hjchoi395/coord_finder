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

// 고해상도 시간 측정 함수 (나노초 단위 반환)
static double current_time_ns() {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    // 나노초 단위로 변환
    return (double)counter.QuadPart * 1e9 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
#endif
}

int main() {
    char input[64], value[128];

    init_cache();
    load_cache();  // 이전 실행 시 저장된 캐시 로드

    while (1) {
        printf(">> Enter base name (or 'exit', 'log'): ");
        if (scanf("%63s", input) != 1) break;

        // 종료 명령
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // 캐시 로그 출력 명령
        if (strcmp(input, "log") == 0) {
            printf("---- CACHE CONTENT ----\n");
            print_cache();
            printf("-----------------------\n");
            continue;
        }

        double t0 = current_time_ns();

        if (cache_lookup(input, value)) {
            double t1 = current_time_ns();
            printf("CACHE HIT : %s -> %s (%.0f ns)\n",
                   input, value, t1 - t0);
        } else {
            if (db_lookup(input, value)) {
                cache_insert(input, value);
                double t1 = current_time_ns();
                printf("CACHE MISS: loaded from DB -> %s (%.0f ns)\n",
                       value, t1 - t0);
            } else {
                double t1 = current_time_ns();
                printf("NOT FOUND  : %s (%.0f ns)\n",
                       input, t1 - t0);
            }
        }
    }

    save_cache();
    printf("Final cache state:\n");
    print_cache();

    return 0;
}
