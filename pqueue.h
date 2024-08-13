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

typedef struct scoreNode {
    float value;
    struct scoreNode *next;
} scoreNode;

typedef struct {
    scoreNode *head;
    int size;
} highscoreQueue;

PriorityQueue* createQueue(void);
highscoreQueue* createScoreQueue(void);
void destroyScoreQueue(highscoreQueue *scoreQueue);
void destroyQueue(PriorityQueue *pq);
bool isEmpty(PriorityQueue *pq);
bool insertQueue(PriorityQueue *pq, const char *data, int wrong);
void removeQueue(PriorityQueue *pq);
const char* getQuestion(PriorityQueue *pq);
void shuffleAnswers(QASet **answers, int count);
void prtAnswers(PriorityQueue *pq);
bool checkAnswer(PriorityQueue*pq, int input, int answer);
void insertHighScoreQueue(highscoreQueue *scoreQueue, int value);

#endif // PRIORITY_QUEUE_H
