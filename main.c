#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define MAX_LINE_LEN 200 //max chars in a line in the q/a set file
#define NUM_CHOICES 4

//maybe hard code these in later, I just put them here to make it easier to calibrate
#define TIME_WEIGHTING 10
#define PERCENTAGE_WEIGHTING 10
#define CORRECT_WEIGHTING 50
#define INCORRECT_PENALTY 100

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

typedef struct scoreNode {
    int value;
    struct scoreNode *next;
} scoreNode;

typedef struct {
    scoreNode *head;
    int size;
} highscoreQueue;

PriorityQueue* createQueue(void) {
    PriorityQueue *pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if(pq == NULL) {
        return NULL;
    }
    pq->head = NULL;
    pq->size = 0;
    return pq;
}

highscoreQueue* createScoreQueue(void) {
    highscoreQueue *scoreQueue = (highscoreQueue*)malloc(sizeof(highscoreQueue));
    if(scoreQueue == NULL) {
        return NULL;
    }
    scoreQueue->head = NULL;
    scoreQueue->size = 0;
    return scoreQueue;
}

void destroyScoreQueue(highscoreQueue *scoreQueue) {
    if(scoreQueue == NULL) {
        return;
    }
    scoreNode *current = scoreQueue->head;
    while (current != NULL) {
        scoreNode *temp = current;
        current = current->next;
        free(temp);
    }
    free(scoreQueue);
    return;
}

void destroyQueue(PriorityQueue *pq) {
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
    return;
}

bool isEmpty(PriorityQueue *pq) {
    return (pq == NULL || pq->size == 0);
}

void insertQueue(PriorityQueue *pq, const char *string, int wrong) {
    QASet *newNode = (QASet*) malloc(sizeof(QASet));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // get the length of both question and answer
    int questionLen = 0;
    while(string[questionLen] != '|' && string[questionLen] != '\0') {
        questionLen++;
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
    } else if(pq->head->next == NULL) {
        pq->head->next = newNode;// prevent repeats when there are only 2 nodes in the queue
    } else {
        QASet *current = pq->head;
        while(current->next != NULL && newNode->timesWrong < current->next->timesWrong) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
    pq->size++;
}


void removeQueue(PriorityQueue *pq) {
    if(pq->head == NULL) {
        return;
    }
    if(pq->size == 1){
        free(pq->head->question);
        free(pq->head->answer);
        free(pq->head);
        pq->head = NULL;
    } else {
        QASet *old_head = pq->head;
        pq->head = pq->head->next;
        free(old_head->question);
        free(old_head->answer);
        free(old_head);
    }
    pq->size -= 1;
}

int getSize(PriorityQueue *pq) {
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
    printf("e: Exit to score breakdown\n\n");
}

bool checkAnswer(QASet *answers[], int correctIndex, int input) {
    return strcmp(answers[input - 1]->answer, answers[correctIndex]->answer) == 0;
}

// void insertHighscoreQueue(highscoreQueue *scoreQueue, int value) {
//     scoreNode *newNode = (scoreNode*) malloc(sizeof(scoreNode));
//     if (newNode == NULL) {
//         fprintf(stderr, "Memory allocation failed\n");
//         exit(1);
//     }

//     newNode->value = value;
//     newNode->next = NULL;

//     if(scoreQueue->head == NULL) {
//         scoreQueue->head = newNode;
//     } else {
//         QASet *current = pq->head;
//         while(current->next != NULL && newNode->timesWrong < current->next->timesWrong) {
//             current = current->next;
//         }
//         newNode->next = current->next;
//         current->next = newNode;
//     }
//     scoreQueue->size++;
// }

int main() {
    time_t startTime = time(NULL);
    PriorityQueue *pq = createQueue();
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
        insertQueue(pq, line, 2); //starts on 2 timesWrong
    }
    fclose(file);

    srand(time(NULL)); //random number generation


    char toInsert[MAX_LINE_LEN];
    int wrong = -1;
    int finalRight = 0;
    int finalWrong = 0;
    
    while(!isEmpty(pq)) {
        printf("%s (%d to complete)\n", getQuestion(pq), pq->head->timesWrong);

        int realNUM_CHOICES = pq->size;
        if(realNUM_CHOICES >= 4) {
            realNUM_CHOICES = NUM_CHOICES;
        }
        QASet *answers[realNUM_CHOICES];
        QASet *current = pq->head;

        for(int i = 0; i < realNUM_CHOICES && current != NULL; i++) {
            answers[i] = current;
            current = current->next;
        }

        shuffleAnswers(answers, realNUM_CHOICES);

        // Find the index of the correct answer in the shuffled array
        int correctIndex = -1;
        for(int i = 0; i < realNUM_CHOICES; i++) {
            if(strcmp(answers[i]->answer, pq->head->answer) == 0) {
                correctIndex = i;
                break;
            }
        }
        
        prtAnswers(answers, realNUM_CHOICES);
        printf("Your answer: ");
        
        char charInput = '\0';
        scanf(" %c", &charInput); // remember space infront of %c to account for newline in input buffer
        if(charInput == 'E' || charInput == 'e') {
            printf("Exiting program...('e' pressed)\n");
            system("clear");
            break;
        }

        int input = charInput - '0';
        if(input < 1 || input > realNUM_CHOICES || input > pq->size) {
            printf("Invalid choice. Please enter a valid option");
            continue;
        }

        if(checkAnswer(answers, correctIndex, input)) {
            system("clear");// same as typing "clear" into command line!
            printf("Correct!\n");
            pq->head->timesWrong -= 1;
            finalRight++;
        } else {
            system("clear");
            printf("Incorrect.\n");
            if (pq->head->timesWrong <= 3) {
                pq->head->timesWrong += 1;
            }
            finalWrong++;
        }

        if(pq->head->timesWrong <= 0) {
            removeQueue(pq);
        } else {
            snprintf(toInsert, sizeof(toInsert), "%s|%s", pq->head->question, pq->head->answer); //creates a string, quesion|answer (auto formatted with \0)
            wrong = pq->head->timesWrong;
            removeQueue(pq);
            insertQueue(pq, toInsert, wrong);
        }
    }
    if(isEmpty(pq)) {
        printf("Exiting program...(no more questions!)\n");
    }
    time_t endTime = time(NULL);
    int secondsElapsed = difftime(endTime, startTime);
    int hoursTotal = (secondsElapsed/3600); 
    int minutesTotal = (secondsElapsed -(3600*hoursTotal))/60;
    int secondsTotal = (secondsElapsed -(3600*hoursTotal)-(minutesTotal*60));
    float percentage = finalRight * 100.0/(finalWrong+finalRight);
    float score = (percentage*(TIME_WEIGHTING/secondsElapsed)) + (percentage*PERCENTAGE_WEIGHTING) + (finalRight*CORRECT_WEIGHTING) - (finalWrong*INCORRECT_PENALTY);
    if(score < 0) {
        score = 0;
        printf("YOU ARE A DUMBASS -Dominic M\n");
    }

    FILE *highscores = fopen("highscores.txt", "a");
    if(highscores == NULL) {
        printf("Could not open file\n");
        return 1;
    }

    fprintf(highscores, "%f\n", score);

    PriorityQueue *scoreQueue = createQueue();
    if(scoreQueue == NULL) {
        fprintf(stderr, "Failed to create priority queue\n");
        return 1;
    }

    while(fgets(line, sizeof(line), highscores)) { // remember this terminates reading whenever it gets to a newline character
        insertQueue(scoreQueue, line, 0);
    }
    printf("the first node is: %s\n", scoreQueue->head->timesWrong);

    // following that, use your knowledge from s1 to sort it into order the most efficient way!

    // finally, search for the inserted score in thed queue/array and assign the position to be the index where it is

    fclose(highscores);

    if(finalRight + finalWrong != 0) {
        printf("You got %d questions wrong, and %d right. you had a %0.2f%% success rate!\n", finalWrong, finalRight, percentage);
        printf("The total time to complete was %d h:%d m:%d s.\n", hoursTotal, minutesTotal, secondsTotal);
        printf("Your score was: %.0f (Position #NUMBER)\n", score); // add position here
        printf("Please enter 'e' to exit the program, or 'h' to view the leaderboard: ");
    } else {
        printf("You didn't even attempt the quiz smh...\n");
        printf("Your total time to let everyone down was: %d h:%d m:%d s.\n", hoursTotal, minutesTotal, secondsTotal);
        printf("Please enter 'e' to exit the program you disappointment: ");
    }

    char exitOrHigh = '\0';
    
    while(exitOrHigh != 'E' && exitOrHigh != 'e') {
        scanf(" %c", &exitOrHigh); // remember space infront of %c to account for newline in input buffer
        if(exitOrHigh == 'E' || exitOrHigh == 'e') {
            printf("Exiting program...\n");
            sleep(1);
            system("clear");
        } else if(exitOrHigh == 'H' || exitOrHigh == 'h') {
            // show highscores here!
            // print the top 10(?) values in the sorted array/queue after each other
            printf("gonna be a highscore table here soon enough lol\n");
        }
    }
    
    free(pq);
    return 0;
}
