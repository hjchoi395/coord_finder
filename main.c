#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
#endif

#include "cache.h"
#include "db.h"
#include "counter.h"

// 고해상도 시간 측정 함수 (나노초 단위)
static double current_time_ns(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1e9 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
#endif
}

int main(void) {
    char input[64], value[128];

    // 1) 캐시 초기화 및 로드
    init_cache();
    load_cache();

    // 2) 카운터 초기화 및 로드
    counter_init();
    load_counter();

    while (1) {
        printf("\n>> Enter command (lookup key / cache / counter / exit): ");
        if (scanf("%63s", input) != 1) {
            // 입력 실패 시 종료
            break;
        }

        // exit
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // cache 보기
        if (strcmp(input, "cache") == 0) {
            printf("\n---- CACHE CONTENT ----\n");
            print_cache();
            printf("-----------------------\n");
            continue;
        }

        // counter 보기
        if (strcmp(input, "counter") == 0) {
            printf("\n---- CALL COUNT HISTORY ----\n");
            counter_print_all();
            printf("-----------------------------\n");
            continue;
        }

        // 그 외엔 좌표 조회
        // 조회 시도마다 카운터를 증가시킨다 (hit/miss 모두 포함)
        counter_increment(input);

        double t0 = current_time_ns();

        // 1) 캐시 조회
        if (cache_lookup(input, value)) {
            double t1 = current_time_ns();
            printf("CACHE HIT : \"%s\" -> \"%s\" (%.0f ns)\n",
                   input, value, t1 - t0);
        }
        // 2) 캐시 미스 → DB 조회
        else if (db_lookup(input, value)) {
            cache_insert(input, value);
            double t1 = current_time_ns();
            printf("CACHE MISS: Load from DB -> \"%s\" (%.0f ns)\n",
                   value, t1 - t0);
        }
        // 3) 미발견
        else {
            double t1 = current_time_ns();
            printf("NOT FOUND  : \"%s\" (%.0f ns)\n",
                   input, t1 - t0);
        }

        // 위 출력 이후 다음 루프 진행
    }

    // 종료 직전: 캐시 저장 및 출력
    save_cache();
    printf("\nFinal cache state:\n");
    print_cache();

    // 카운터 저장, 출력, 메모리 해제
    save_counter();
    printf("\nFinal call counts:\n");
    counter_print_all();
    counter_free_all();

    return 0;
}
