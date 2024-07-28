// REMEMBER TO ADD A NEWLINE CHAR AFTER LAST QUESTION!!!!!!

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pqueue.h"

#define MAX_LINE_LEN 200

PriorityQueue* pq_create(void) {
    PriorityQueue *pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if(pq == NULL) {
        return NULL;
    }
    pq->head = NULL;
    pq->size = 0;
    return pq;
}

void pq_destroy(PriorityQueue *pq) {
    if(pq == NULL) {
        return;
    }
    struct QASet *current = pq->head;
    while (current != NULL) {
        struct QASet *temp = current;
        current = current->next;
        free(temp->question);
        free(temp->answer);
        free(&temp->timesWrong);
        free(temp);       // Free the node
    }
    free(pq); // Free the priority queue structure
}

bool pq_is_empty(PriorityQueue *pq) {
    return (pq == NULL || pq->size == 0);
}

// MAKE CLEANER
bool pq_insert(PriorityQueue *pq, const char *string) {
    struct QASet *newNode = (struct QASet*) malloc(sizeof(struct QASet));
    if(newNode == NULL) {
        return false;
    }

    int questionLen = 0;
    while(string[questionLen] != '|'){
        questionLen ++;
    }

    int answerLen = questionLen;
    while(string[answerLen] != '\0'){
        answerLen ++;
    }
    answerLen = strlen(string) - questionLen;

    char onlyQuestion[questionLen + 1];
    char onlyAnswer[answerLen + 1];

    strncpy(onlyQuestion, string, questionLen);
    onlyQuestion[questionLen] = '\0';

    strncpy(onlyAnswer, string + questionLen + 1, answerLen);
    onlyAnswer[answerLen - 2] = '\0';

    newNode->question = strdup(onlyQuestion);
    newNode->answer = strdup(onlyAnswer);
    newNode->timesWrong = 0;
    newNode->next = NULL;

    if(pq->head == NULL) {
        pq->head = newNode;
        pq->size += 1;
        return true;
    } else {
        struct QASet *prev = NULL;
        struct QASet *current = pq->head;
        while(current != NULL && current->timesWrong >= newNode->timesWrong) {
            prev = current;
            current = current->next;
        }
        newNode->next = current;
        prev->next = newNode;
        pq->size += 1;
        return true;
    }
}

void pq_remove(PriorityQueue *pq) {
    struct QASet *old_head = pq->head;
    pq->head = pq->head->next;
    char *tempQuestion = old_head->question;
    char *tempAnswer = old_head->answer;
    int tempWrong = old_head->timesWrong;
    
    free(tempQuestion);
    free(tempAnswer);
    //free(tempWrong);
    free(old_head);
    pq->size -= 1;

    return;
}

int pq_size(PriorityQueue *pq) {
    return pq ? pq->size : 0;
}

const char* getQuestion(PriorityQueue *pq) {
    return pq->head->question;
}

void prtAnswers(PriorityQueue *pq) {
    char *first = pq->head->answer;
    char *second = pq->head->next->answer;
    char *third = pq->head->next->next->answer;
    char *fourth = pq->head->next->next->next->answer;

    //print these four strings in a random order
}

bool checkAnswer(PriorityQueue*pq, int input) {
    return true;
}

int main(){
    int input = 0;

    PriorityQueue *pq = pq_create();
    if(pq == NULL) {
        fprintf(stderr, "Failed to create priority queue\n");
        return 1;
    }

    FILE *file;
    char ch;
    char line[MAX_LINE_LEN];

    file = fopen("questions.txt", "r");

    if(file == NULL) {
        printf("Could not open file\n");
        return 1;
    }
    
    while(fgets(line, sizeof(line), file)) {
        pq_insert(pq, line);
    }
    
    while(input != 5) {
        printf("%s\n", getQuestion(pq));
        prtAnswers(pq);
        printf("Your answer: ");
        scanf("%d", &input);
        if(checkAnswer(pq, input) == true) {
            printf("Correct!\n");
        } else {
            printf("Incorrect.\n");
            pq->head->timesWrong += 1;
        }
        //if timesWrong is -1, remove from pq
        //else, remove from pq and reinsert back into list at correct spot
    }
    printf("exiting program...\n");

    return 0;
}
