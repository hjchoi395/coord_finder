#ifndef CACHE_H
#define CACHE_H

#include <time.h>
#include <stdbool.h>

#define MAX_CACHE_SIZE   100
#define PERSIST_FILE     "cache_state.db"
// TTL: 14일(초)
#define TTL_SEC          (14 * 24 * 60 * 60)

// 캐시 엔트리 구조체 (이중 연결 리스트 및 TTL 관리)
typedef struct CacheEntry {
    char key[64];
    char value[128];
    time_t timestamp;              // 저장 시각 기록
    struct CacheEntry *prev;
    struct CacheEntry *next;
} CacheEntry;

// 초기화
void init_cache(void);

// 조회: hit이면 out_value에 복사 후  true, miss이면 false 반환
bool cache_lookup(const char *key, char *out_value);

// 삽입: 기존 엔트리 갱신 또는 신규 추가
// (용량 초과 시 LRU 꼬리 제거)
void cache_insert(const char *key, const char *value);

// 프로그램 종료 전 TTL 만료되지 않은 엔트리만 파일에 저장
void save_cache(void);

// 프로그램 시작 시 파일에서 엔트리 로드 (TTL 검사)
void load_cache(void);

// 디버그용: 현재 캐시 전체 내용 출력
void print_cache(void);

#endif