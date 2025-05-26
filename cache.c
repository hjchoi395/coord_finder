#include "cache.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// 리스트 헤드/테일 포인터 및 카운트
static CacheEntry *head = NULL;
static CacheEntry *tail = NULL;
static int cache_count = 0;

// TTL 만료 검사
static bool is_expired_timestamp(time_t ts) {
    time_t now = time(NULL);
    return (now - ts) > TTL_SEC;
}

// 엔트리 제거 (연결 해제 + 메모리 해제)
static void remove_entry(CacheEntry *e) {
    if (!e) return;
    if (e->prev) e->prev->next = e->next;
    else          head = e->next;
    if (e->next) e->next->prev = e->prev;
    else          tail = e->prev;
    free(e);
    cache_count--;
}

// 엔트리를 리스트 맨 앞으로 이동 (MRU)
static void move_to_head(CacheEntry *e) {
    if (e == head) return;
    // unlink
    if (e->prev) e->prev->next = e->next;
    if (e->next) e->next->prev = e->prev;
    if (e == tail) tail = e->prev;
    // insert at head
    e->prev = NULL;
    e->next = head;
    if (head) head->prev = e;
    head = e;
}

// 캐시 초기화
void init_cache(void) {
    head = tail = NULL;
    cache_count = 0;
}

bool cache_lookup(const char *key, char *out_value) {
    CacheEntry *e = head;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            // TTL expired?
            if (is_expired_timestamp(e->timestamp)) {
                CacheEntry *expired = e;
                e = e->next;  // advance before removal
                remove_entry(expired);
                return false;
            }
            // 성공: 값 복사, MRU로 이동, 타임스탬프 갱신
            strcpy(out_value, e->value);
            e->timestamp = time(NULL);
            move_to_head(e);
            return true;
        }
        e = e->next;
    }
    return false;
}

void cache_insert(const char *key, const char *value) {
    // 기존 엔트리 갱신
    CacheEntry *e = head;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            strcpy(e->value, value);
            e->timestamp = time(NULL);
            move_to_head(e);
            return;
        }
        e = e->next;
    }
    // 신규 엔트리 생성 & 머리 삽입
    CacheEntry *new_e = malloc(sizeof(CacheEntry));
    if (!new_e) return;
    strncpy(new_e->key,   key,   sizeof(new_e->key)-1);
    new_e->key[sizeof(new_e->key)-1] = '\0';
    strncpy(new_e->value, value, sizeof(new_e->value)-1);
    new_e->value[sizeof(new_e->value)-1] = '\0';
    new_e->timestamp = time(NULL);

    new_e->prev = NULL;
    new_e->next = head;
    if (head) head->prev = new_e;
    head = new_e;
    if (!tail) tail = new_e;
    cache_count++;

    // 용량 초과 시 LRU 제거
    if (cache_count > MAX_CACHE_SIZE) {
        CacheEntry *old = tail;
        tail = old->prev;
        if (tail) tail->next = NULL;
        free(old);
        cache_count--;
    }
}

void save_cache(void) {
    FILE *fp = fopen(PERSIST_FILE, "w");
    if (!fp) return;
    CacheEntry *e = head;
    time_t now = time(NULL);
    // MRU부터 순서대로 저장
    while (e) {
        if ((now - e->timestamp) <= TTL_SEC) {
            fprintf(fp, "%ld %s %s\n",
                    (long)e->timestamp,
                    e->key,
                    e->value);
        }
        e = e->next;
    }
    fclose(fp);
}

void load_cache(void) {
    FILE *fp = fopen(PERSIST_FILE, "r");
    if (!fp) return;

    // 기존 캐시 비우기 (필요시)
    init_cache();

    long ts;
    char key[sizeof(((CacheEntry*)0)->key)];
    char value[sizeof(((CacheEntry*)0)->value)];
    time_t now = time(NULL);

    // 파일에 기록된 순서(MRU→…→LRU) 그대로
    // 매 줄마다 tail에 직접 추가하여 순서 보존
    while (fscanf(fp, "%ld %63s %127[^\n]", &ts, key, value) == 3) {
        if ((now - ts) <= TTL_SEC) {
            CacheEntry *new_e = malloc(sizeof(CacheEntry));
            if (!new_e) continue;

            // 데이터 복사 & 원본 타임스탬프 복원
            strncpy(new_e->key,   key,   sizeof(new_e->key)-1);
            new_e->key[sizeof(new_e->key)-1] = '\0';
            strncpy(new_e->value, value, sizeof(new_e->value)-1);
            new_e->value[sizeof(new_e->value)-1] = '\0';
            new_e->timestamp = (time_t)ts;

            // 리스트 끝(tail)에 연결
            new_e->next = NULL;
            new_e->prev = tail;
            if (tail)        tail->next = new_e;
            else             head = new_e;
            tail = new_e;
            cache_count++;
        }
    }

    fclose(fp);
    remove(PERSIST_FILE);
}

void print_cache(void) {
    printf("Cache Contents (count=%d):\n", cache_count);
    CacheEntry *e = head;
    time_t now = time(NULL);
    while (e) {
        printf("  %s -> %s (age=%ld sec)\n",
               e->key,
               e->value,
               (long)(now - e->timestamp));
        e = e->next;
    }
}