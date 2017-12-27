/****************************************
    Author: Tsewei Peng
****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LEN 2048
#define MAX_ARG_LEN 512

char** getArgs(int*, int*);
int checkBackground(char**, int*);
char* getInput();
char* expandBuffer(char*, int*);
char** parseArgs(char*, int*);
char** expandArgs(char**, int*);
void freeArgs(char**);

// This function is where we get user input and split the string into arguments for commands
char** getArgs(int* backgroundStatus, int* numArgs) {
    char* buffer = getInput();
    char** args = parseArgs(buffer, numArgs);
    // If the input is empty return
    if (args[0] == NULL) {
        *backgroundStatus = -1;
    }
    // Check if it is a backgrond process by &
    else {
        *backgroundStatus = checkBackground(args, numArgs);
    }
    free(buffer);
    return args;
}

int checkBackground(char** args, int* numArgs) {
    if (*numArgs >= 1) {
        if(strcmp(args[*numArgs - 1], "&") == 0) {
            return 1;
        }
    }
    return 0;
}

char* getInput() {
    char c;
    char* buffer = malloc(sizeof(char) * 8);
    int position = 0;
    int bufferCap = 8;

    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;
        if (position == bufferCap) {
            if (bufferCap < MAX_INPUT_LEN) {
                buffer = expandBuffer(buffer, &bufferCap);
            }
            else {
                printf("\nBuffer limit reached.\n");
                break;
            }
        }
    }
}

// This function expand the char array if it is too small
char* expandBuffer(char* buffer, int* bufferCap) {
    int i = 0;
    char* newBuffer = malloc(sizeof(char) * *bufferCap * 2);
    for (i = 0; i < *bufferCap; i++) {
        newBuffer[i] = buffer[i];
    }
    free(buffer);
    buffer = newBuffer;
    *bufferCap *= 2;
    return buffer;
}

// This function parse the input into arguments
char** parseArgs(char* buffer, int* numArgs) {
    int i = 0, j = 0;
    char** args = malloc(sizeof(char*) * 4);
    int argsCap = 4, startPos = 0;
    *numArgs = 0;
    while (i < strlen(buffer)) {
        // Whenever the previous char is a space, set the startPos and curr position
        if (buffer[i - 1] == ' ' && i > 0){
            startPos = i;
        }
        // If next is space, then the argument is found, process the argument
        if (buffer[i + 1] == ' ' || buffer[i + 1] == '\0') {
            char* newArg = malloc(sizeof(char) * (i - startPos + 2));
            for (j = startPos; j <= i; j++) {
                newArg[j - startPos] = buffer[j];
            }
            newArg[i - startPos + 1] = '\0';
            args[*numArgs] = newArg;
            (*numArgs)++;
            if (*numArgs == argsCap) {
                args = expandArgs(args, &argsCap);
            }
        }
        i++;
    }
    args[*numArgs] = NULL;
    return args;
}

// This function is to expand the number of arguments args hold.
char** expandArgs(char** args, int* argsCap) {
    int i = 0;
    char** newArgs = malloc(sizeof(char*) * *argsCap * 2);
    for (i = 0; i < *argsCap; i++) {
        newArgs[i] = args[i];
    }
    free(args);
    args = newArgs;
    *argsCap *= 2;
    return args;
}

void freeArgs(char** args) {
    int i = 0;
    while (args[i] != NULL) {
        free(args[i]);
        i++;
    }
    free(args);
}
