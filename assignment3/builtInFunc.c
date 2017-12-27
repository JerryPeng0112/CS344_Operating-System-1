/****************************************
    Author: Tsewei Peng
****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int buildInFunc(char**, int*, int);
void clearBuiltInArgs(char** args, int* numArgs);
int cdCommand(char**, int*);
int statusCommand(int*, int);
int exitCommand(char**, int*);

// Where all the built-in cammands are called
int builtInFunc(char** args, int* numArgs, int builtInStatus) {
    clearBuiltInArgs(args, numArgs);
    int status = 0;
    switch (args[0][0]) {
        // Comment
        case '#':
        status = 1;
            break;
        // cd
        case 'c':
            status = cdCommand(args, numArgs);
            break;
        // status
        case 's':
            status = statusCommand(numArgs, builtInStatus);
            break;
        // exit
        case 'e':
            status = exitCommand(args, numArgs);
            break;
    }
    return status;
}

// This function clear up the & sign if it exists
void clearBuiltInArgs(char** args, int* numArgs) {
    if (strcmp(args[*numArgs - 1], "&") == 0) {
        free(args[*numArgs - 1]);
        args[*numArgs - 1] = NULL;
        (*numArgs)--;
    }
}

int cdCommand(char** args, int* numArgs) {
    if (*numArgs > 2) {
        return 1;
    }
    // If cd, go back home
    if (*numArgs == 1) {
        chdir(getenv("HOME"));
    }
    // Otherwise go to the directory
    else {
        char cwd[256];
        char newDir[256];
        getcwd(cwd, sizeof(cwd));
        if (strcmp(args[1], "..") != 0) {
            sprintf(newDir, "%s/%s", cwd, args[1]);
        }
        else {
            strcpy(newDir, "..");
        }
        if (chdir(newDir) != 0) {
            printf("Directory does not exist\n");
        }
    }
    return 0;
}

int statusCommand(int* numArgs, int status) {
    if(*numArgs > 1) {
        return 1;
    }
    if(status == -1) {
        printf("exit value 1\n");
        return 1;
    }
    else {
        printf("exit value %d\n", status);
        return status;
    }
}

int exitCommand(char** args, int* numArgs) {
    freeArgs(args);
    exit(0);
}
