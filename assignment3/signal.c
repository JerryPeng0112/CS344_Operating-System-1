/****************************************
    Author: Tsewei Peng
****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static pid_t FOREGROUND;
static int RUNFOREGROUND = 0;

// The signal handler for SIGINT
void catchSIGINTchild(int signo) {
    int exitValue;
    waitpid(FOREGROUND, &exitValue, WCONTINUED);
    char* buffer = malloc(sizeof(char) * 100);
    sprintf(buffer, "terminated by signal %d\n", signo);
    write(STDOUT_FILENO, buffer, 100);
    free(buffer);
}

/*void catchSIGTSTP(int signo) {
    char* buffer = "Entering foreground-only mode (& is now ignored)\n";
    write(STDOUT_FILENO, buffer, 100);
    free(buffer);
    RUNFOREGROUND = 1 - RUNFOREGROUND;
}*/
