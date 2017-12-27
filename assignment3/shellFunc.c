/****************************************
    Author: Tsewei Peng
****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "signal.c"

int shellFUnc(char**, int, int*);
int redirect(char**, int*, int, int*, int*, int*, int*);
int countSigns(char**, int*, int, int*, int*);
void clearArgs(char**, int*, int*, int*);
int foregroundProcess(char**);
void expandPid(char**, pid_t);
int backgroundProcess(char**);

// Where non built-in functions run
int shellFunc(char** args, int backgroundStatus, int* numArgs) {

    int status = 0, dirInput = 0, dirOutput = 0, result = 0;
    int rfd, wfd;
    // saving original file descriptor for stdin, stdout
    int stdin_save = dup(0);
    int stdout_save = dup(1);
    // count if the arguments are valid (">, <, &")
    if (countSigns(args, numArgs, backgroundStatus, &dirInput, &dirOutput) != 0) {
        return -1;
    }
    // Redirect the files if there are any < or > sign
    result = redirect(args, numArgs, backgroundStatus, &dirInput, &dirOutput, &rfd, &wfd);
    if (result != 0) {
        return result;
    }
    // Clear the arguments of < or > arguments
    clearArgs(args, numArgs, &dirInput, &dirOutput);
    if (backgroundStatus == 0 || RUNFOREGROUND == 1) {
        // Run foreground process
        status = foregroundProcess(args);
    }
    else {
        // run background process
        status = backgroundProcess(args);
    }
    // Restore stdin, stdout file descriptor
    dup2(stdin_save, 0);
    dup2(stdout_save, 1);
    return status;
}

void clearArgs(char** args, int* numArgs, int* dirInput, int* dirOutput) {
    if (strcmp(args[*numArgs - 1], "&") == 0) {
        free(args[*numArgs - 1]);
        args[*numArgs - 1] = NULL;
        (*numArgs)--;
    }
    if (*dirOutput == 1) {
        free(args[*numArgs - 2]);
        free(args[*numArgs - 1]);
        args[*numArgs - 2] = NULL;
        (*numArgs) -= 2;
    }
    if (*dirInput == 1) {
        free(args[*numArgs - 2]);
        free(args[*numArgs - 1]);
        args[*numArgs - 2] = NULL;
        (*numArgs) -= 2;
    }
}

int redirect(char** args, int* numArgs, int backgroundStatus, int* dirInput, int* dirOutput, int* rfd, int* wfd) {
    int index = *numArgs, result;
    if (backgroundStatus != 0) {
        index--;
    }
    // If there are > sign, redirect to the file name
    if (*dirOutput == 1) {
        index--;
        *wfd = open(args[index], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*wfd == -1) { printf("cannot open %s for input\n", args[index]); return 1; }
        result = dup2(*wfd, 1);
        if (result == -1) { perror("source dup2()"); return 1; }
        index--;
        close(*wfd);
    }
    // If there are < sign, redirect to the file name
    if (*dirInput == 1) {
        index --;
        *rfd = open(args[index], O_RDONLY);
        if (*rfd == -1) { printf("cannot open %s for output\n", args[index]); return 1; }
        result = dup2(*rfd, 0);
        if (result == -1) { perror("source dup2()"); return 1; }
        index--;
        close(*rfd);
    }
    return 0;
}

// This function count the number of <, > signs. If there are abnormalities, return a status -1
int countSigns(char** args, int* numArgs, int backgroundStatus, int* dirInput, int* dirOutput) {
    int i = 0, index = *numArgs;
    if (backgroundStatus != 0) {
        index--;
    }
    for (i = 0; i < *numArgs; i++) {
        if (strcmp(args[i], "<") == 0) {
            (*dirInput)++;
        }
        if (strcmp(args[i], ">") == 0) {
            (*dirOutput)++;
        }
    }
    if (*dirInput > 1 || *dirOutput > 1) {
        return -1;
    }
    if (*dirInput == 1 && *dirOutput == 0 && strcmp(args[index - 2], "<") != 0) {
        return -1;
    }
    if (*dirInput == 0 && *dirOutput == 1 && strcmp(args[index - 2], ">") != 0) {
        return -1;
    }
    if (*dirInput == 1 && *dirOutput == 1) {
        if (strcmp(args[index - 4], "<") != 0 || strcmp(args[index - 2], ">") != 0) {
            return -1;
        }
    }
    return 0;
}

int foregroundProcess(char** args) {
    // The sigaction struct for SIGINT handling
    struct sigaction SIG_endForegroundchild = {0};
    SIG_endForegroundchild.sa_handler = catchSIGINTchild;
    sigaddset(&SIG_endForegroundchild.sa_mask, SIGINT);
    SIG_endForegroundchild.sa_flags = 0;

    pid_t spawnPid = -5;
    int childExitStatus = -5;
    spawnPid = fork();
    switch (spawnPid) {
        case -1: {
            perror("Hull Breach\n");
            return 1;
        }
        case 0: {
            // Child Process
            // Below is expanding $$ to process id
            expandPid(args, getpid());
            FOREGROUND = getpid();
            // Call to ensure the signal handle work
            sigaction(SIGINT, &SIG_endForegroundchild, NULL);
            if (execvp(args[0], args) != 0) {
                return -1;
            }
            return 0;
            break;
        }
        default: {
            // Parent process
            // Call the same signal handler for SIGINT
            sigaction(SIGINT, &SIG_endForegroundchild, NULL);
            pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
            return 0;
            break;
        }
    }
}

// This function take the PID, find any $$ sign, and expand it into the PID within
// The args variable
void expandPid(char** args, pid_t spawnPid) {
    int i = 0, length = 0, j = 0;
    char* pch;
    char* temp;
    while (args[i] != NULL) {
        pch = strstr(args[i], "$$");
        if(pch != NULL) {
            length = strlen(args[i]);
            char* temp1 = malloc(sizeof(char) * (length + 5));
            char* temp2 = malloc(sizeof(char) * (length + 5));
            char* buffer = malloc(sizeof(char) * (length + 5));
            while (args[i] + sizeof(char) * j != pch) {
                temp1[j] = args[i][j];
                j++;
            }
            temp1[j] = '\0';
            j += 2;
            int mark = j;
            while (args[i][j] != '\0') {
                temp2[j - mark] = args[i][j];
                j++;
            }
            temp2[j - mark] = '\0';
            sprintf(buffer, "%s%d%s", temp1, (int) spawnPid, temp2);
            free(temp1);
            free(temp2);
            free(args[i]);
            args[i] = buffer;
        }
        i++;
    }
}

int backgroundProcess(char** args) {
    pid_t spawnPid = -5;
    int childExitStatus = -5;
    spawnPid = fork();
    switch (spawnPid) {
        case 1: {
        perror("Hull Breach\n");
        return 1;
        }
        case 0: {
            // child process
            expandPid(args, getpid());
            if (execvp(args[0], args) != 0) {
                return -1;
            }
            return 0;
            break;
        }
        default: {
            // Parent process
            printf("background pid is %d\n", spawnPid);
            return 0;
            break;
        }
    }
    return 0;
}
