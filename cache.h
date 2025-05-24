#ifndef CACHE_H
#define CACHE_H

#define MAX_CACHE_SIZE   100
#define PERSIST_FILE     "cache_state.db"

typedef struct {
    char key[64];
    char value[128];
} CacheEntry;

// 초기화
void init_cache(void);

// 조회: hit이면 value에 복사하고 1, miss이면 0 반환
int  cache_lookup(const char *key, char *out_value);

// 삽입: 이미 있으면 갱신+앞으로, 새 항목이면 맨 앞에 추가(용량 초과 시 마지막 항목 제거)
void cache_insert(const char *key, const char *value);

// 프로그램 종료 전 캐시 상태를 파일에 저장
void save_cache(void);

// 프로그램 시작 시 파일에서 캐시 상태 로드
void load_cache(void);

// 디버그용: 현재 캐시 전체 내용 출력
void print_cache(void);

#endif
