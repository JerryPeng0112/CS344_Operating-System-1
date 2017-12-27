/*
    Author: Tsewei Peng
    Filename: otp_dec.c
    Date: 03/18/2017
    Description: Client for OTP decryption
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

// Define the length of buffer
#define BUFFERLENGTH 1024

int inputValidation(char*, char*, char*);
int verify(int);
int sendText(int, char*, char*);
char* getStringFromFile(char*);
char* expandTextCap(char*, int*);
void error(const char*);



int main(int argc, char* argv[]) {
    int socketFD, portNumber, status = 0;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;

    // If the argument is less than 4, exit with value 1
    if (argc < 4) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Set up the server address struct, and port number
    memset((char*) &serverAddress, '\0', sizeof(serverAddress));
    portNumber = atoi(argv[3]);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverHostInfo = gethostbyname("localhost");

    if(serverHostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    memcpy((char*) &serverAddress.sin_addr.s_addr, (char*) serverHostInfo->h_addr, serverHostInfo->h_length);


    // Get the to-be-encrypted string from file
    char* text = getStringFromFile(argv[1]);
    // Get the key from file
    char* key = getStringFromFile(argv[2]);

    // If the text and key is not valid, return error message with exit status 1
    if (inputValidation(text, key, argv[2]) == 1) {
        return 1;
    }


    // Create socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: error on socket()");
    }
    // connect socket to server socket
    if (connect(socketFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: error on connect()");
    }

    // Verify the server is enc_d server, output error if not the correct server
    status = verify(socketFD);
    if (status == 2) {
        fprintf(stderr, "Error: could not contact otp_dec_d on port %d\n", portNumber);
        return 2;
    }

    // If all the connection established, send the strings
    status = sendText(socketFD, text, key);
    return status;
}

/*
    Function Name: inputValidation
    Argument: char* text, char* key, char* keyfile (filename)
    Return Type: int (status)
    Description: Check if all the input is valid
*/
int inputValidation(char* text, char* key, char* keyfile) {
    int i = 0;

    // If the key is shorter than text, return error
    if (strlen(text) > strlen(key)) {
        fprintf(stderr, "Error: key ‘%s’ is too short\n", keyfile);
        return 1;
    }

    // If the text contains invalid characters, return error
    for (i = 0; i < strlen(text); i++) {
        if ((text[i] < 'A' || text[i] > 'Z') && text[i] != ' ') {
            fprintf(stderr, "otp_dec_d error: input contains bad characters\n");
            return 1;
        }
    }
    // If the key contains invalid characters, return error
    for (i = 0; i < strlen(key); i++) {
        if ((key[i] < 'A' || key[i] > 'Z') && key[i] != ' ') {
            fprintf(stderr, "otp_dec_d error: input contains bad characters\n");
            return 1;
        }
    }
    // If all tests passed, return 0 to indicate correct status
    return 0;
}

/*
    Function Name: verify()
    Argument: int (socket file descriptor)
    Return Type: int (status)
    Description: Send text to server, and receive text to verify server
*/
int verify(int socketFD) {
    int charsWritten, charsRead;

    // Set the send verification text to "e"
    char sendBuffer[2] = "d";
    char recvBuffer[2];
    do {
        charsWritten = send(socketFD, sendBuffer, 2, 0);
        if (charsWritten == 2) {
            // In this "if" statement if text sent successfully
            do {
                charsRead = recv(socketFD, recvBuffer, 2, 0);
                if (charsRead != 2) {
                    error("CLIENT: error on verification send()");
                }
                else {
                    // In this "else" statement if received successfully
                    if (strcmp(recvBuffer, "y") == 0) {
                        // If the message received is "y", accept connection
                        return 0;
                    } else {
                        // If the message received is not "y", drop connection
                        return 2;
                    }
                }
            } while (1);
        } else {
            error("CLIENT: error on verification send()");
        }
    } while (1);
}

/*
    Function Name: sentText()
    Argument: int socketFD (socket file descriptor), char* text, char* key
    Return Type: int (status)
    Description: Send the text and key to server to encrypt
                & receive the cipher text
*/
int sendText(int socketFD, char* text, char* key) {
    int i = 0, charsWritten, charsRead;
    // Set up C string variable for merged text to send to server & cipher text received
    char* cipherText = malloc(sizeof(char) * (strlen(text) + 1));
    memset(cipherText, '\0', strlen(text) + 1);
    char* mergedText = malloc(sizeof(char) * (strlen(text) + strlen(key) + 5));

    // Merge the text and key with terminal characters and put into mergedText
    sprintf (mergedText, "%s@@%s##", text, key);

    // Set up buffer
    char buffer[BUFFERLENGTH + 1];
    memset(buffer, '\0', BUFFERLENGTH + 1);

    // Copy chunks to buffer, then send, until whole mergedText is sent
    while (i < strlen(mergedText)) {
        buffer[i % BUFFERLENGTH] = mergedText[i];

        // If the buffer is full, or reach the end of mergedText, send it to server
        if ((i + 1) % BUFFERLENGTH == 0 || mergedText[i + 1] == '\0') {
            charsWritten = send(socketFD, buffer, BUFFERLENGTH, 0);
            if (charsWritten < 0) {
                error("CLIENT: error on send()");
                return 1;
            }
            // Reset buffer
            memset(buffer, '\0', BUFFERLENGTH + 1);
        }
        i++;
    }
    // Reset "i" counter
    i = 0;

    // Receive chunks to buffer, then concat to cipherText
    while (i < strlen(text)) {
        memset(buffer, '\0', BUFFERLENGTH + 1);
        charsRead = recv(socketFD, buffer, BUFFERLENGTH, 0);
        if (charsRead <= 0) {
            error("Error on recv()");
            break;
        }
        strcat(cipherText, buffer);
        i += strlen(buffer);
    }

    // output the cipherText
    printf("%s\n", cipherText);
    return 0;
}

/*
    Function Name: getStringFromFile
    Argument: char* filename
    Return Type: char*
    Description: Get the string from file
*/
char* getStringFromFile(char* filename) {
    int i = 0;
    char c;
    int textCap = BUFFERLENGTH;
    char* text = malloc(sizeof(char) * (textCap + 1));
    FILE* file = fopen(filename, "r");

    // In the while loop, get character from file and add it to string
    while ((c = getc(file)) != '\n') {
        if (i == textCap) {
            // If string limit reached, expand the string
            text = expandTextCap(text, &textCap);
        }
        text[i] = c;
        i++;
    }
    if (strlen(text) > textCap - 2) {
        text = expandTextCap(text, &textCap);
    }
    // Close the file
    fclose(file);
    return text;
}

/*
    Function Name: expandTextCap
    Argument: char* text, int* textCap (capacity of the C string)
    Return Type: char*
    Description: Expand the capacity of the C string
*/
char* expandTextCap(char* text, int* textCap) {
    int i = 0;
    // Setup a new string with double capacity
    char* newText = malloc(sizeof(char) * (*textCap * 2 + 1));
    // Copy the characters from old string to new string
    for (i = 0; i < *textCap; i++) {
        newText[i] = text[i];
    }
    free(text);
    *textCap *= 2;
    return newText;
}

void error(const char *msg) {
    perror(msg); exit(1);
}
