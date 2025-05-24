#include "cache.h"
#include <stdio.h>
#include <string.h>

static CacheEntry cache[MAX_CACHE_SIZE];
static int cache_count = 0;

void init_cache(void) {
    cache_count = 0;
}

int cache_lookup(const char *key, char *out_value) {
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i].key, key) == 0) {
            // 값 복사
            strcpy(out_value, cache[i].value);
            // LRU: 이 엔트리를 맨 앞으로 이동
            CacheEntry tmp = cache[i];
            memmove(&cache[1], &cache[0], i * sizeof(CacheEntry));
            cache[0] = tmp;
            return 1;
        }
    }
    return 0;
}

void cache_insert(const char *key, const char *value) {
    // 1) 이미 존재하면 값 갱신 + 맨 앞으로
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i].key, key) == 0) {
            strcpy(cache[i].value, value);
            CacheEntry tmp = cache[i];
            memmove(&cache[1], &cache[0], i * sizeof(CacheEntry));
            cache[0] = tmp;
            return;
        }
    }
    // 2) 신규 항목 삽입
    if (cache_count < MAX_CACHE_SIZE) {
        // 비어 있는 만큼만 shift
        memmove(&cache[1], &cache[0], cache_count * sizeof(CacheEntry));
        cache_count++;
    } else {
        // 용량 초과 시 마지막 요소(LRU)를 덮고 shift
        memmove(&cache[1], &cache[0], (MAX_CACHE_SIZE - 1) * sizeof(CacheEntry));
    }
    // 새 엔트리 세팅
    strncpy(cache[0].key,   key,   sizeof(cache[0].key)-1);
    cache[0].key[sizeof(cache[0].key)-1] = '\0';
    strncpy(cache[0].value, value, sizeof(cache[0].value)-1);
    cache[0].value[sizeof(cache[0].value)-1] = '\0';
}

void save_cache(void) {
    FILE *fp = fopen(PERSIST_FILE, "w");
    if (!fp) return;
    for (int i = 0; i < cache_count; i++) {
        fprintf(fp, "%s %s\n", cache[i].key, cache[i].value);
    }
    fclose(fp);
}

void load_cache(void) {
    FILE *fp = fopen(PERSIST_FILE, "r");
    if (!fp) return;

    char key[64], value[128];
    // "%63s" 와 "%127[^\n]" 조합으로 키와 값(공백 포함)을 안전하게 읽음
    while (cache_count < MAX_CACHE_SIZE
           && fscanf(fp, "%63s %127[^\n]", key, value) == 2) {
        // 현재 save_cache는 MRU부터 순서대로 저장하므로
        // load 시 insert()를 쓰면 순서가 뒤집히지 않고 그대로 복원됨
        cache_insert(key, value);
    }
    fclose(fp);
    remove(PERSIST_FILE);
}

void print_cache(void) {
    printf("Cache Contents (count=%d):\n", cache_count);
    for (int i = 0; i < cache_count; i++) {
        printf("  [%2d] %s -> %s\n", i, cache[i].key, cache[i].value);
    }
}