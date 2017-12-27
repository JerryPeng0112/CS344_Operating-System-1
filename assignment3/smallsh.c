/****************************************
    Author: Tsewei Peng
****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "getArgs.c"
#include "builtInFunc.c"
#include "shellFunc.c"

void checkProcess(int);
void outputError(char**, int);

void main() {
    /*struct sigaction SIG_foregroundMode = {0};
    SIG_foregroundMode.sa_handler = catchSIGTSTP;
    sigaddset(&SIG_foregroundMode.sa_mask, SIGTSTP);
    SIG_foregroundMode.sa_flags = 0;*/

    // Initialize status as 0
    int status = 0;
    // Running the shell
    while (1) {
        int backgroundStatus = 0;
        int numArgs = 0;
        printf(": ");

        //sigaction(SIGTSTP, &SIG_foregroundMode, NULL);

        // Get the command arguments
        char** args = getArgs(&backgroundStatus, &numArgs);
        // If the arguments are not empty
        if(backgroundStatus != -1) {
            // Built-in functions
            if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "exit") == 0 ||
            strcmp(args[0], "status") == 0 || args[0][0] == '#') {
                status = builtInFunc(args, &numArgs, status);
            }
            else {
                // Non built-in functions
                status = shellFunc(args, backgroundStatus, &numArgs);
            }
        }
        // Output the error
        outputError(args, status);
        // Check for background processes
        checkProcess(backgroundStatus);
        // Free the arrays of char*
        freeArgs(args);
    }
}

void outputError(char** args, int status) {
    if (status == -1) {
        printf("%s: Command not valid\n", args[0]);
    }
}

void checkProcess(int backgroundStatus) {
    pid_t tempPid = -1;
    int childExitStatus = -5, result = 0;
    // Output all the background processes
    do {
        result = waitpid(tempPid, &childExitStatus, WNOHANG);
        if(result > 0 && backgroundStatus == 1) {
            // If exited normally, print this
            if (WIFEXITED(childExitStatus) != 0) {
                printf("background pid %d is done: exit value %d\n", result, childExitStatus);
            }
            // If exited through kill(), print this
            if (WIFSIGNALED(childExitStatus) != 0) {
                printf("background pid %d is done: terminated by signal %d\n", result, childExitStatus);
            }
        }
    } while (result > 0);
}
