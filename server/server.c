
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "server.h"
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_SIZE 256
#define MAX_CLIENTS 2

static void uploadFile(int sd, FILE *stream, char *buffer, char *fileBuffer);
static void readFile(FILE *stream, char *buffer, char *fileBuffer);
static void writeFile(int sd, FILE *stream, char *buffer, char *fileBuffer);

void createServer()
{
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLen;

    int opt = 1;
    int sock, newSock, clientSock[MAX_CLIENTS];
    char *hostName = "192.168.68.59";
    int portNo = 80;

    char buffer[BUFFER_SIZE];
    char fileBuffer[BUFFER_SIZE];

    int n;
    FILE *stream, *fopen();
    char *message = "Please press enter to download hello.txt\r\n\0";

    fd_set readfds;
    int max_sd, sd, activity, valread;
    int max_clients = MAX_CLIENTS;
    memset(clientSock, 0, sizeof(clientSock));

    // Get socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error opening socket");
        exit(0);
    }

    // Sets socket option at socket level: reuse addresses supplied to bind
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("Error setsockopt");
        exit(0);
    }

    // Initialise server
    memset((char *)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(hostName);
    serverAddress.sin_port = htons(portNo);

    // Bind to the socket
    if (bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error on binding");
        exit(0);
    }

    // Listen to the client up to 2 pending connections
    if (listen(sock, 2) < 0)
    {
        printf("Error on listening");
        exit(0);
    }
    printf("Listening for client connection\n");

    // Accept a connection on the socket and creates a new socket
    clientAddressLen = sizeof(clientAddress);

    while (1)
    {
        FD_ZERO(&readfds);      // clear the socket set
        FD_SET(sock, &readfds); // add master socket to set
        max_sd = sock;
        // Add multiple clients' sockets to the set
        for (int i = 0; i < max_clients; i++)
        {
            sd = clientSock[i];
            if (sd > 0)
            {
                FD_SET(sd, &readfds); // add valid socket descriptor to read list
            }
            // Limit the number of sockets used
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL); // monitor multiple file descriptors waiting until one more is ready
        if ((activity < 0))
        {
            printf("Error on monitoring connection");
            exit(0);
        }

        // Incoming connection
        if (FD_ISSET(sock, &readfds))
        {
            printf("Incoming connection\n");
            newSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLen);
            if (newSock < 0)
            {
                printf("Error on accepting connection");
                exit(0);
            }

            // send message to download a file
            if (send(newSock, message, strlen(message), 0) != strlen(message))
            {
                printf("Error sending message");
            }

            // Add new socket if the list is available
            for (int i = 0; i < max_clients; i++)
            {
                if (clientSock[i] == 0)
                {
                    clientSock[i] = newSock;
                    printf("new client[%d] added: %d\n\n", i, clientSock[i]);
                    break;
                }
            }
        }
        // Other IO operation. new data or disconnection
        for (int i = 0; i < max_clients; i++)
        {
            sd = clientSock[i];
            if (FD_ISSET(sd, &readfds)) // tests to see if a file descriptor is part of the set
            {
                memset(buffer, 0, BUFFER_SIZE);
                valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0)
                {
                    //  printf("buffer %d\n\n", buffer[0]);
                    getpeername(sd, (struct sockaddr *)&serverAddress, (socklen_t *)&clientAddressLen);
                    close(sd);
                    clientSock[i] = 0; // close the socket and mark 0 for reuse
                }
                else
                {
                    uploadFile(sd, stream, buffer, fileBuffer);
                }
            }
        }
    }
    memset(fileBuffer, 0, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);

    printf("Exit Server\n");
    sleep(5);
    fclose(stream);
    close(newSock);
    close(sock);
}

void uploadFile(int sd, FILE *stream, char *buffer, char *fileBuffer)
{
    printf(buffer); // TODO check if valid name
    readFile(stream, buffer, fileBuffer);
    writeFile(sd, stream, buffer, fileBuffer);
}

void readFile(FILE *stream, char *buffer, char *fileBuffer)
{ // Read the file content
  //  stream = fopen(buffer, "r"); // TODO pass the file name
    stream = fopen("hello.txt", "r");
    if (stream == NULL)
    {
        printf("Error in opening file %s\n", buffer);
        exit(0);
    }
    fscanf(stream, "%s", fileBuffer);
    printf("reading file content: %s\n", fileBuffer);
    //  writeFile(sd, stream, buffer, fileBuffer);
}

void writeFile(int sd, FILE *stream, char *buffer, char *fileBuffer)
{ // Write file content to the client
    int n = write(sd, fileBuffer, strlen(fileBuffer));
    if (n < 0)
    {
        printf("Error writing to the socket\n");
        return;
    }
    printf("sending file %s\n", fileBuffer);
}

/*
#include <netinet/in.h>

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};
*/
/*
#include <netdb.h>

struct  hostent {
     char *  h_name;
     char ** h_aliases;
     int     h_addrtype;
     int     h_length;
     char ** h_addr_list;
};
#define h_addr  h_addr_list[0]
*/