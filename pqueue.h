#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>

struct QASet{
    char *question;
    char *answer;
    int timesWrong;
    struct QASet *next;
};

typedef struct{
    struct QASet *head;
    int size;
} PriorityQueue;

// NEW FUNCTIONS TO ADD: random 4 questions (inc. correct one), peek at top node

PriorityQueue* pq_create(void);
void pq_destroy(PriorityQueue *pq);
bool pq_insert(PriorityQueue *pq, const char *data);
void pq_remove(PriorityQueue *pq);
bool pq_is_empty(PriorityQueue *pq);
int pq_size(PriorityQueue *pq);

#endif // PRIORITY_QUEUE_H
