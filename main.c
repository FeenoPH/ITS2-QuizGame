#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_LINE_LEN 200 //max chars in a line in the q/a set file
#define NUM_CHOICES 4 //maybe have the ability to select "difficulties" with different number of choices?
#define TIME_WEIGHTING 2
#define CORRECT_WEIGHTING 100
#define INCORRECT_PENALTY 50

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
    float value;
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
    // Ignore first line of file (question set title)
    if (string[0] == '-') {
        return;
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

void insertHighScoreQueue(highscoreQueue *scoreQueue, int value) {
    scoreNode *newNode = (scoreNode*) malloc(sizeof(scoreNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    newNode->value = value;
    newNode->next = NULL;

    if(scoreQueue->head == NULL) {
        scoreQueue->head = newNode;
    } else if(newNode->value > scoreQueue->head->value) {
        newNode->next = scoreQueue->head;
        scoreQueue->head = newNode;
    } else {
        scoreNode *current = scoreQueue->head;
        while(current->next != NULL && newNode->value <= current->next->value) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
    scoreQueue->size++;
}

char selectQuestionset() {
    DIR *questionsetDirectory;
    char *directoryName = "questionsets";
    int questionSetsTotal = 0;
    questionsetDirectory = opendir(directoryName);

    struct dirent *dp;
    dp = readdir(questionsetDirectory);
    int fileNum = 1;
    printf("Select Question Set: \n");
    while (dp != NULL){
        char filePath[101];
        if (dp->d_type == DT_REG) {
            sprintf(filePath, "%s%s", "questionsets/", dp->d_name);
            questionSetsTotal++;
            FILE *file = fopen(filePath, "r");
            char line[MAX_LINE_LEN];
            fgets(line, sizeof(line), file);
            printf("%i: %s\n", fileNum, line);
            fclose(file);
            fileNum++;
        }
        dp = readdir(questionsetDirectory);
    }
    closedir(questionsetDirectory);
    printf("E: Exit Program\n");
    printf("Your Selection: ");
    char charInput = '\0';
    scanf(" %c", &charInput); // remember space infront of %c to account for newline in input buffer
    int input = charInput - '0';
    if(input < 1 || input > questionSetsTotal) {
        system("clear");
        printf("Invalid choice.\n");
        return -1;
    }
    else {
        system("clear");
        return charInput;
    }
}

void editQuestionsets() {

}

int runQuiz() {
    char qSet = selectQuestionset();
    time_t startTime = time(NULL);
    PriorityQueue *pq = createQueue();
    if(pq == NULL) {
        fprintf(stderr, "Failed to create priority queue\n");
        return 1;
    }
    char filePath[101];
    sprintf(filePath, "%s%c%s", "questionsets/questions", qSet, ".txt");
    FILE *file = fopen(filePath, "r");
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
            system("clear");
            printf("Invalid choice. Please enter a valid option\n");
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
    
    float score = 0;
    if(secondsElapsed == 0 || finalRight == 0) {
        score = 0;
    } else {
        score = (((secondsElapsed * TIME_WEIGHTING)/finalRight) * CORRECT_WEIGHTING) + 2 * ((finalRight * CORRECT_WEIGHTING) - (finalWrong * INCORRECT_PENALTY));
        if(score < 0) {
            score = 0;
        }
    }

    FILE *highscores = fopen("highscores.txt", "a");
    if(highscores == NULL) {
        printf("Could not open file\n");
        return 1;
    }

    fprintf(highscores, "%f\n", score);
    fclose(highscores); // close and reopen in "read" mode

    highscores = fopen("highscores.txt", "r");
    if(highscores == NULL) {
        printf("Could not open file\n");
        return 1;
    }

    highscoreQueue *scoreQueue = createScoreQueue();
    if(scoreQueue == NULL) {
        fprintf(stderr, "Failed to create priority queue\n");
        return 1;
    }

    float currentScore = 0;
    while(fscanf(highscores, "%f", &currentScore) == 1) {
        //printf("got: %f\n", currentScore);
        insertHighScoreQueue(scoreQueue, currentScore);
    }

    fclose(highscores);
    
    int index = 1;
    scoreNode *indexFinder = scoreQueue->head;
    while(score != indexFinder->value) {
        indexFinder = indexFinder->next;
        index++;
    }

    if(finalRight + finalWrong != 0) {
        printf("You got %d questions wrong, and %d right. you had a %0.2f%% success rate!\n", finalWrong, finalRight, percentage);
        printf("The total time to complete was %d h:%d m:%d s.\n", hoursTotal, minutesTotal, secondsTotal);
        printf("Your score was: %.0f (Position #%d out of %d attempts)\n", score, index, scoreQueue->size); // add position here
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
            printf("\n\nTop 10 scores:\n\n");
            scoreNode *leaderboardTraverse = scoreQueue->head;
            for(int i = 1; i < 11; i++) {
                printf("%d. %.0f\n", i, leaderboardTraverse->value);
                leaderboardTraverse = leaderboardTraverse->next;
            }
            printf("\n\nPlease enter 'e' to exit the program, or 'h' to view the leaderboard: ");
        }
    }
    
    destroyQueue(pq);
    destroyScoreQueue(scoreQueue);
    return 0;
}

int main() {
    printf("Welcome to the Quiz Game!\n\nPlease select the mode you wish to open the Quiz game in:\n");
    printf("1: Play quiz\n");
    printf("2: Edit Questionsets\n");
    printf("E: Exit program\n");
    char charInput = '\0';
    scanf(" %c", &charInput); // remember space infront of %c to account for newline in input buffer
    if(charInput == 'E' || charInput == 'e') {
        printf("Exiting program...('e' pressed)\n");
        sleep(1);
        system("clear");
        return 0;
    }
    else if(charInput == '1') {
        system("clear");
        runQuiz();
    }
    else if(charInput == '2') {
        system("clear");
        editQuestionsets();
        return 0;
    }
    else {
        printf("Invalid Input\n");
        return 1;
    }

}
