#include <stdio.h>
#include <string.h>
#include <time.h>

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
// ※ double 대신 int64_t 기반으로 추출해 정밀도 보강 가능
static double current_time_ns() {
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

int main() {
    char input[64], value[128];

    // 1) 캐시 초기화 및 로드
    init_cache();
    load_cache();

    // 2) 카운터 초기화 및 로드
    counter_init();
    load_counter();

    while (1) {
        printf("\n>> Enter command (coordinate lookup: enter key / cache: view cache / "
            "counter: view call history / exit: quit): ");
        if (scanf("%63s", input) != 1) {
            // 입력이 더 이상 없으면 종료
            break;
        }

        // 종료
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // 캐시 내용 출력
        if (strcmp(input, "cache") == 0) {
            printf("\n---- CACHE CONTENT ----\n");
            print_cache();
            printf("-----------------------\n");
            continue;
        }

        // 카운터(호출 기록) 내용 출력
        if (strcmp(input, "counter") == 0) {
            printf("\n---- CALL COUNT HISTORY ----\n");
            counter_print_all();
            printf("-----------------------------\n");
            continue;
        }

        // 그 외의 입력은 “좌표 조회” 로 간주
        // (1) 카운트 집계
        counter_increment(input);

        // (2) 조회 시간 측정
        double t0 = current_time_ns();

        if (cache_lookup(input, value)) {
            double t1 = current_time_ns();
            printf("CACHE HIT : \"%s\" -> \"%s\" (%.0f ns)\n",
                   input, value, t1 - t0);
        } else {
            if (db_lookup(input, value)) {
                cache_insert(input, value);
                double t1 = current_time_ns();
                printf("CACHE MISS: Load from DB -> \"%s\" (%.0f ns)\n",
                       value, t1 - t0);
            } else {
                double t1 = current_time_ns();
                printf("NOT FOUND  : \"%s\" (%.0f ns)\n",
                       input, t1 - t0);
            }
        }
    }

    // 프로그램 종료 직전: 캐시 저장 → 캐시 내용 출력
    save_cache();
    printf("\nFinal cache state:\n");
    print_cache();

    // 카운터 저장 → 호출 기록 출력 → 메모리 해제
    save_counter();
    printf("\nFinal call counts:\n");
    counter_print_all();
    counter_free_all();

    return 0;
}