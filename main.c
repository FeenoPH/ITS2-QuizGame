#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LINE_LEN 200 //max chars in a line in the q/a set file
#define NUM_CHOICES 4

typedef struct QASet {
    char *question;
    char *answer;
    int timesWrong;
    struct QASet *next;
} QASet;

typedef struct {
    QASet *head;
    int size;
} PriorityQueue;

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
    QASet *current = pq->head;
    while (current != NULL) {
        QASet *temp = current;
        current = current->next;
        free(temp->question);
        free(temp->answer);
        free(temp);
    }
    free(pq);
}

bool pq_is_empty(PriorityQueue *pq) {
    return (pq == NULL || pq->size == 0);
}

bool pq_insert(PriorityQueue *pq, const char *string, int wrong) {
    QASet *newNode = (QASet*) malloc(sizeof(QASet));
    if(newNode == NULL) {
        return false;
    }

    int questionLen = 0;
    while(string[questionLen] != '|' && string[questionLen] != '\0'){
        questionLen ++;
    }

    int answerLen = strlen(string) - questionLen - 1;

    char onlyQuestion[questionLen + 1];
    char onlyAnswer[answerLen + 1];

    strncpy(onlyQuestion, string, questionLen);
    onlyQuestion[questionLen] = '\0';

    strncpy(onlyAnswer, string + questionLen + 1, answerLen);
    onlyAnswer[answerLen] = '\0';

    newNode->question = strdup(onlyQuestion);
    newNode->answer = strdup(onlyAnswer);
    newNode->timesWrong = wrong;
    newNode->next = NULL;

    if(pq->head == NULL) {
        pq->head = newNode;
    } else {
        QASet *prev = NULL;
        QASet *current = pq->head;
        while(current != NULL && current->timesWrong >= newNode->timesWrong) {
            prev = current;
            current = current->next;
        }
        newNode->next = current;
        if(prev == NULL) {
            pq->head = newNode;
        } else {
            prev->next = newNode;
        }
    }
    pq->size += 1;
    return true;
}

void pq_remove(PriorityQueue *pq) {
    if(pq->head == NULL) {
        return;
    }
    QASet *old_head = pq->head;
    pq->head = pq->head->next;
    free(old_head->question);
    free(old_head->answer);
    free(old_head);
    pq->size -= 1;
}

int pq_size(PriorityQueue *pq) {
    return pq ? pq->size : 0;
}

const char* getQuestion(PriorityQueue *pq) {
    return pq->head ? pq->head->question : "No more questions.";
}

void shuffleAnswers(QASet **answers, int count) {
    if (count <= 1) return;

    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        QASet *temp = answers[i];
        answers[i] = answers[j];
        answers[j] = temp;
    }
}

void prtAnswers(QASet *answers[], int count) {
    for (int i = 0; i < count; i++) {
        printf("%d: %s", i + 1, answers[i]->answer);
    }
    printf("E: exit program\n\n");
}

bool checkAnswer(QASet *answers[], int correctIndex, int input) {
    return strcmp(answers[input - 1]->answer, answers[correctIndex]->answer) == 0;
}

int main() {
    PriorityQueue *pq = pq_create();
    if(pq == NULL) {
        fprintf(stderr, "Failed to create priority queue\n");
        return 1;
    }

    FILE *file = fopen("questions.txt", "r");
    if(file == NULL) {
        printf("Could not open file\n");
        return 1;
    }
    
    char line[MAX_LINE_LEN];
    while(fgets(line, sizeof(line), file)) {
        pq_insert(pq, line, 0);
    }
    fclose(file);

    srand(time(NULL)); //random number generation

    while(!pq_is_empty(pq)) {
        printf("%s\n", getQuestion(pq));

        QASet *answers[NUM_CHOICES];
        QASet *current = pq->head;
        for (int i = 0; i < NUM_CHOICES && current != NULL; i++) {
            answers[i] = current;
            current = current->next;
        }

        shuffleAnswers(answers, NUM_CHOICES);

        // Find the index of the correct answer in the shuffled array
        int correctIndex = -1;
        for (int i = 0; i < NUM_CHOICES; i++) {
            if (strcmp(answers[i]->answer, pq->head->answer) == 0) {
                correctIndex = i;
                break;
            }
        }

        prtAnswers(answers, NUM_CHOICES);

        printf("Your answer: ");
        
        char charInput = '\0';
        scanf(" %c", &charInput);
        if (charInput == 'E' || charInput == 'e') {
            printf("Exiting program...('e' pressed)\n");
            break;
        }

        int input = charInput - '0';
        if (input < 1 || input > NUM_CHOICES) {
            printf("Invalid choice. Please enter a number between 1 and %d.\n", NUM_CHOICES);
            continue;
        }

        if (checkAnswer(answers, correctIndex, input)) {  // Use the correct index
            system("clear");
            printf("Correct!\n");
            pq->head->timesWrong -= 1;
        } else {
            system("clear");
            printf("Incorrect.\n");
            pq->head->timesWrong += 1;
        }

        if(pq->head->timesWrong < 0) {
            pq_remove(pq);
        } else {
            char toInsert[MAX_LINE_LEN];
            snprintf(toInsert, sizeof(toInsert), "%s|%s", pq->head->question, pq->head->answer);

            int wrong = pq->head->timesWrong;
            pq_remove(pq);
            pq_insert(pq, toInsert, wrong);
        }
    }

    printf("Exiting program...(no more questions!)\n");
    pq_destroy(pq);
    return 0;
}

