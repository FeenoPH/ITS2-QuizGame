#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>

typedef struct QASet{
    char *question;
    char *answer;
    int timesWrong;
    struct QASet *next;
} QASet;

typedef struct {
    QASet *head;
    int size;
} PriorityQueue;

// NEW FUNCTIONS TO ADD: random 4 questions (inc. correct one), peek at top node

PriorityQueue* pq_create(void);
void pq_destroy(PriorityQueue *pq);
bool pq_is_empty(PriorityQueue *pq);
bool pq_insert(PriorityQueue *pq, const char *data, int wrong);
void pq_remove(PriorityQueue *pq);
int pq_size(PriorityQueue *pq);
const char* getQuestion(PriorityQueue *pq);
void prtAnswers(PriorityQueue *pq);
bool checkAnswer(PriorityQueue*pq, int input, int answer);

#endif // PRIORITY_QUEUE_H
