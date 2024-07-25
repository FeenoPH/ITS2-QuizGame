// REMEMBER TO ADD A NEWLINE CHAR AFTER LAST QUESTION!!!!!!

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pqueue.h"

#define MAX_LINE_LEN 200

// Create a priority queue
PriorityQueue* pq_create(void) {
    PriorityQueue *pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if(pq == NULL) {
        return NULL;
    }
    pq->head = NULL;
    pq->size = 0;
    return pq;
}

// Destroy the priority queue
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

// Check if the priority queue is empty
bool pq_is_empty(PriorityQueue *pq) {
    return (pq == NULL || pq->size == 0);
}

// Insert a question/answer node into the pqueue based on timesWrong
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

// Remove the most wrong question
void pq_remove(PriorityQueue *pq) {
    struct QASet *old_head = pq->head;
    pq->head = pq->head->next;
    char *tempQuestion = old_head->question;
    char *tempAnswer = old_head->answer;
    int tempWrong = old_head->timesWrong;
    
    free(tempQuestion);
    free(tempAnswer);
    free(tempWrong);
    free(old_head);
    pq->size -= 1;

    return;
}

// Get the current size of the priority queue
int pq_size(PriorityQueue *pq) {
    return pq ? pq->size : 0;
}

int main(){
    PriorityQueue *pq = pq_create();
    if(pq == NULL) {
        fprintf(stderr, "Failed to create priority queue\n");
        return 1;
    }

    FILE *file;
    char ch;
    char line[MAX_LINE_LEN];

    // Open the file in read mode
    file = fopen("questions.txt", "r");

    // Check if file was successfully opened
    if(file == NULL) {
        printf("Could not open file\n");
        return 1;
    }
    
    while(fgets(line, sizeof(line), file)) {
        pq_insert(pq, line);
    }
    
    printf("%s\n", pq->head->question);
    printf("%s\n", pq->head->answer);
    printf("%d\n", pq->head->timesWrong);
    printf("%s\n", pq->head->next->question);
    printf("%s\n", pq->head->next->answer);
    printf("%d\n", pq->head->next->timesWrong);
    printf("%s\n", pq->head->next->next->question);
    printf("%s\n", pq->head->next->next->answer);
    printf("%d\n", pq->head->next->next->timesWrong);
    return 0;
}
