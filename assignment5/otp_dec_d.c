/*
    Author: Tsewei Peng
    Filename: otp_dec_d.c
    Date: 03/18/2017
    Description: Server for OTP decryption
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Define the length of buffer
#define BUFFERLENGTH 1024
// Define the global variable: number of child processes
static int numChildren = 0;

int decrypt(int);
char* oneTimeDecryption(char*);
char* generateDecipher(char*, char*);
int verify(int);
char* expandTextCap(char*, int*);
void SIGCHLDHandler(int);
void error(const char*);

int main(int argc, char* argv[]) {
    // Define signal handler for SIGCHLD when child processes terminates
    struct sigaction sa = {0};
    sa.sa_handler = &SIGCHLDHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);
    int listenSocketFD, newConnectionFD, portNumber, status;
    socklen_t sizeOfClientInfo;
    struct sockaddr_in serverAddress, clientAddress;

    if (argc < 2) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Set up the server address struct, and port number
    portNumber = atoi(argv[1]);
    memset((char*) &serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Set up the listen socket file descriptor
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocketFD < 0) {
        error("Error opening socket");
        exit(1);
    }

    // Bind the socket to server address
    if (bind(listenSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        error("Error on bind()");
        exit(1);
    }

    // Listen on the socket
    listen(listenSocketFD, 5);

    // The loop will make sure the server runs permanently
    while (1) {
        sizeOfClientInfo = sizeof(clientAddress);

        // Make the new connection file descriptor if a client is accepted connection
        newConnectionFD = accept(listenSocketFD, (struct sockaddr*) &clientAddress, &sizeOfClientInfo);
        if (newConnectionFD > 0) {
            // In this "if" statement if new connection accepted
            // Create a child process using fork
            pid_t spawnPid = fork();
            switch (spawnPid) {
                case 1:
                    error("Error on fork()");
                    break;
                case 0:
                    // Child proces: decrypt the code
                    status = decrypt(newConnectionFD);
                    close(newConnectionFD);
                    exit(status);
                default:
                    // Increase the cound of number of chil processes
                    numChildren++;
                    close(newConnectionFD);
                    break;
            }
        }
    }
    close(listenSocketFD);
}

/*
    Function Name: decrypt()
    Argument: int new connection file descriptor
    Return type: int
    Description: Decrypt the text with the key received,
                & then send the decipher text back to client
*/
int decrypt(int newConnectionFD) {
    // Verify the client, if not verify, return function and drop connection
    if(verify(newConnectionFD) == 2) return 2;

    int status = 0, charsRead, charsWritten, textCap = BUFFERLENGTH, i = 0;
    char* text = malloc(sizeof(char) * (BUFFERLENGTH + 1));

    // Set up the buffer
    char buffer[BUFFERLENGTH + 1];

    // Receive the text from client, keep receiving until the termination characters are found
    while (strstr(text, "##") == NULL) {
        // If the text capacity is about to be reached, expand the capacity
        if (strlen(text) > textCap - 4) {
            text = expandTextCap(text, &textCap);
        }
        // Reset the buffer
        memset(buffer, '\0', BUFFERLENGTH + 1);

        // Receive the strings from buffer
        charsRead = recv(newConnectionFD, buffer, BUFFERLENGTH, 0);
        if (charsRead <= 0) {
            error("Error on recv()");
            break;
        }

        // Concat the buffer to text
        strcat(text, buffer);
    }

    // Get the decipherText from depryption function
    char* decipherText = oneTimeDecryption(text);
    // Reset bufer
    memset(buffer, '\0', BUFFERLENGTH + 1);

    // Copy the decipherText to buffer in chunks, and send them until all texts are sent
    while (i < strlen(decipherText)) {
        buffer[i % BUFFERLENGTH] = decipherText[i];
        // If buffer is full, or the end of text is reached, send the buffer
        if ((i + 1) % BUFFERLENGTH == 0 || decipherText[i + 1] == '\0') {
            charsWritten = send(newConnectionFD, buffer, BUFFERLENGTH, 0);
            if (charsWritten < 0) {
                error("SERVER: error on send()");
                return 1;
            }
            // Reset the buffer
            memset(buffer, '\0', BUFFERLENGTH + 1);
        }
        i++;
    }
}

/*
    Function Name: oneTimeDecryption()
    Argument: char* mergedText
    return value: char*
    Description: split the mergedText into text and key, then decrypt
*/
char* oneTimeDecryption(char* mergedText) {
    int textCap = strlen(mergedText) / 2, keyCap = strlen(mergedText) / 2;
    int numTextChars = 0, numKeyChars = 0, i = 0;
    char* textEnd, * keyEnd;
    // Set up C string text and key
    char* text = malloc(sizeof(char) * textCap);
    char* key = malloc(sizeof(char) * keyCap);
    memset(text, '\0', sizeof(text));
    memset(key, '\0', sizeof(key));
    // Get where text and key ends
    textEnd = strstr(mergedText, "@@");
    keyEnd = strstr(mergedText, "##");
    // Get the number of characters in text and key
    numTextChars = (textEnd - mergedText) / sizeof(char);
    numKeyChars = (keyEnd - textEnd) / sizeof(char) - 2;
    // Copy from merged text to text
    for (i = 0; i < numTextChars; i++) {
        if (i == textCap) {
            text = expandTextCap(text, &textCap);
        }
        text[i] = mergedText[i];
    }
    text[i] = '\0';

    // Copy from merged text to key
    for (i = 0; i < numKeyChars; i++) {
        if (i == keyCap) {
            key = expandTextCap(key, &keyCap);
        }
        key[i] = mergedText[i + numTextChars + 2];
    }
    key[i] = '\0';

    // Generate the decipher text and return
    char* decipherText = generateDecipher(text, key);
    return decipherText;
}

/*
    Function Name: generateDeipher
    Argument: char* text, char* key
    return value: char* cipherText
    Description: Use the text and key to generate decipher text
*/
char* generateDecipher(char* text, char* key) {
    int i = 0, textChar, keyChar, newChar;
    char c;
    char* decipherText = malloc(sizeof(char) * (strlen(text) + 1));
    for(i = 0; i < strlen(text); i++) {

        // Get the index of the character
        if (text[i] == ' ') {
            textChar = 26;
        }
        else {
            textChar = text[i] - 'A';
        }
        if (key[i] == ' ') {
            keyChar = 26;
        }
        else {
            keyChar = key[i] - 'A';
        }
        // Add the index and mod 27
        newChar = textChar - keyChar;
        if (newChar < 0) newChar += 27;
        // Convert back to character
        if (newChar == 26) {
            c = ' ';
        }
        else {
            c = 'A' + newChar;
        }
        decipherText[i] = c;
    }
    decipherText[strlen(text)] = '\0';
    return decipherText;
}

/*
    Function Name: verify()
    Argument: int newConnectionFD (File Descritpor)
    return value: int (status)
    Description: Verify with the client that connection is valid
*/
int verify(int newConnectionFD) {
    int charsRead, charsWritten, verified = 0;
    char recvBuffer[2];
    char sendBuffer[2] = "y";
    do {
        // Receive text from the client
        charsRead = recv(newConnectionFD, recvBuffer, 2, 0);
        if (charsRead != 2) {
            error("SERVER: error on verification recv()");
        }
        else {
            // In this "else" statement if the correct number of char received
            if (strcmp(recvBuffer, "d") == 0) {
                // In this "if" statement if "d" is received, send 'y' to confirm
                do {
                    charsWritten = send(newConnectionFD, sendBuffer, 2, 0);
                    if (charsWritten == 2) {
                        // If text sent successfully
                        return 0;
                    } else {
                        error("SERVER: error on verification send()");
                    }
                } while (1);
            } else {
                // In this "else" statement if "e" is received, send 'n' to reject
                sendBuffer[0] = 'n';
                do {
                    charsWritten = send(newConnectionFD, sendBuffer, 2, 0);
                    if(charsWritten == 2) {
                        // If text send successfully, return with error code 2
                        return 2;
                    }
                    else {
                        error("SERVER: error on verification send()");
                    }
                } while (!verified);

            }
        }
    } while (1);
}

/*
    Function Name: expandTextCap()
    Argument: char* text, int* textCap (text capacity)
    return value: char*
    Description: Expand the capacity of the text
*/
char* expandTextCap(char* text, int* textCap) {
    int i = 0;
    // Create newText with double capacity
    char* newText = malloc(sizeof(char) * (*textCap * 2 + 1));
    // Reset newText
    memset(newText, '\0', *textCap * 2 + 1);
    // Copy from text to newText
    for (i = 0; i < *textCap; i++) {
        newText[i] = text[i];
    }
    free(text);
    *textCap *= 2;
    return newText;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Signal handler for SIGCHLD
void SIGCHLDHandler(int signo) {
    int status = -5;
    // If any child process terminate, decrease the numChildren count
    while (waitpid(-1, &status, WNOHANG) > 0) {
        numChildren--;
    }
    return;
}
