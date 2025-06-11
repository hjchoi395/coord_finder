#include "counter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 전역 리스트 헤드 (초기값 NULL)
CounterEntry *counter_head = NULL;

void counter_init(void) {
    counter_head = NULL;
}

void counter_increment(const char *key) {
    // 기존 entry 검색
    for (CounterEntry *cur = counter_head; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            cur->count++;
            return;
        }
    }

    // 없으면 새 노드 생성
    CounterEntry *new_e = malloc(sizeof *new_e);
    if (!new_e) return;
    strncpy(new_e->key, key, MAX_KEY_LEN);
    new_e->key[MAX_KEY_LEN] = '\0';
    new_e->count = 1;
    new_e->next = counter_head;
    counter_head = new_e;
}

void counter_print_all(void) {
    printf("=== Call Count History ===\n");
    for (CounterEntry *cur = counter_head; cur; cur = cur->next) {
        printf("  \"%s\" -> %dtimes\n", cur->key, cur->count);
    }
    printf("==========================\n");
}

void counter_free_all(void) {
    CounterEntry *cur = counter_head;
    while (cur) {
        CounterEntry *tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    counter_head = NULL;
}

void load_counter(void) {
    FILE *fp = fopen(COUNTER_PERSIST_FILE, "r");
    if (!fp) return;

    counter_init();
    char key[MAX_KEY_LEN+1];
    int cnt;

    // 파일 포맷: "key count\n"
    while (fscanf(fp, "%63s %d", key, &cnt) == 2) {
        CounterEntry *new_e = malloc(sizeof *new_e);
        if (!new_e) break;
        strncpy(new_e->key, key, MAX_KEY_LEN);
        new_e->key[MAX_KEY_LEN] = '\0';
        new_e->count = cnt;
        new_e->next = counter_head;
        counter_head = new_e;
    }

    fclose(fp);
    remove(COUNTER_PERSIST_FILE);
}

void save_counter(void) {
    FILE *fp = fopen(COUNTER_PERSIST_FILE, "w");
    if (!fp) return;

    for (CounterEntry *cur = counter_head; cur; cur = cur->next) {
        fprintf(fp, "%s %d\n", cur->key, cur->count);
    }

    fclose(fp);
}
