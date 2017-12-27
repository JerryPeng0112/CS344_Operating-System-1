/*
    Author: Tsewei Peng
    Filename: keygen.c
    Date: 03/18/2017
    Description: Generate random key for OTP transimission
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[]) {

    // If the command line argument is not 4 (output to file) or not 2 (stdout)
    // Then return
    if (argc != 4 && argc != 2) {
        return 1;
    }
    int i = 0, temp;

    // Get the number of characters requested
    int numChar = atoi(argv[1]);

    // C string for the key
    char* key = malloc(sizeof(char) * (numChar + 2));
    memset(key, '\0', sizeof(key));

    time_t t;
    srand((unsigned) time(&t));

    // Generate characters and then copy it to the C string "key"
    for (i = 0; i < numChar; i++) {
        // Generate random number: 0-25 'A' - 'Z', 26 ' '
        temp = rand() % 27;
        if (temp < 26) {
            key[i] = 'A' + temp;
        }
        else {
            key[i] = ' ';
        }
    }
    // Add newline char
    key[numChar] = '\n';
    // Add null-terminated char
    key[numChar + 1] = '\0';
    printf("%s", key);
    free(key);
    return 0;
}
