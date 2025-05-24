#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
  #include <sys/timeb.h>
#else
  #include <sys/time.h>
  #include <unistd.h>
#endif

#include "cache.h"
#include "db.h"

#define DC_DELAY_MS 100  // data center 왕복 지연 시간 (밀리초)
#define TTL_SEC     30   // 캐시 TTL (초)

// 현재 시각을 ms 단위로 반환
static double current_time_ms() {
#ifdef _WIN32
    struct _timeb tb; _ftime(&tb);
    return tb.time * 1000.0 + tb.millitm;
#else
    struct timeval tv; gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
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
            printf("CACHE HIT : %s -> %s (%.2f ms)\n",
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
                printf("CACHE MISS: loaded from DB -> %s (%.2f ms including ~%d ms DC delay)\n",
                       value, t1 - t0, DC_DELAY_MS);
            } else {
                double t1 = current_time_ms();
                printf("NOT FOUND  : %s (%.2f ms)\n",
                       input, t1 - t0);
            }
        }
    }

    save_cache();   // 종료 시 캐시에 남은 데이터를 파일로 저장
    print_cache();

    return 0;
}