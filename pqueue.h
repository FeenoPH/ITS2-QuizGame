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

PriorityQueue* createQueue(void);
void destroyQueue(PriorityQueue *pq);
bool isEmpty(PriorityQueue *pq);
bool insertQueue(PriorityQueue *pq, const char *data, int wrong);
void removeQueue(PriorityQueue *pq);
int getSize(PriorityQueue *pq);
const char* getQuestion(PriorityQueue *pq);
void prtAnswers(PriorityQueue *pq);
bool checkAnswer(PriorityQueue*pq, int input, int answer);

#endif // PRIORITY_QUEUE_H
