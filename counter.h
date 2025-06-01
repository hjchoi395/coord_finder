#ifndef COUNTER_H
#define COUNTER_H

#include <time.h>

// 최대 키 길이(캐시/DB의 key 길이와 동일)
#define MAX_KEY_LEN 63

// 카운터 영속성 파일 이름
#define COUNTER_PERSIST_FILE "counter_state.db"

// 호출 횟수를 저장할 구조체 (단일 연결 리스트)
typedef struct CounterEntry {
    char key[MAX_KEY_LEN+1];
    int count;
    struct CounterEntry *next;
} CounterEntry;

// 전역 헤드 포인터
extern CounterEntry *counter_head;

// 모듈 초기화
void counter_init(void);

// 특정 key의 카운트를 +1 (없으면 새 노드 생성)
void counter_increment(const char *key);

// 키별 호출 횟수 출력
void counter_print_all(void);

// 프로그램 종료 시 메모리 해제
void counter_free_all(void);

// 영속화된 파일에서 로드
void load_counter(void);

// 종료 시 현재 카운터를 파일에 저장
void save_counter(void);

#endif // COUNTER_H
