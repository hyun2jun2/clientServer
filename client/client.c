
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "client.h"
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256

static void getData(int sock, char *fileBuffer);
static void sendData(int sock, char *buffer);
static void writeFile(char *fileBuffer);
static void dataHandler(int sock, char *buffer, char *fileBuffer);

void createClient()
{
    struct sockaddr_in serverAddress;
    struct hostent *internetHost;

    int sock;
    char *hostName = "192.168.68.59";
    int portNo = 80;

    char buffer[BUFFER_SIZE];
    int n;
    char fileBuffer[BUFFER_SIZE];

    // Get socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error opening socket");
        exit(0);
    }
    internetHost = gethostbyname(hostName);
    if (internetHost == NULL)
    {
        printf("Error getting host name");
        exit(0);
    }
    // puts(internetHost->h_name);

    // Initialise socket
    memset((char *)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    memmove((char *)&serverAddress.sin_addr.s_addr, (char *)internetHost->h_addr, internetHost->h_length);
    // printf("internetHost->h_addr %x %x %x %x\n", internetHost->h_addr[0], internetHost->h_addr[1], internetHost->h_addr[2], internetHost->h_addr[3]); // FFC0 FFA8 44 43 192.168.68.67
    serverAddress.sin_port = htons(portNo);

    // Connect to the Server
    if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error connecting");
        exit(0);
    }

    dataHandler(sock, buffer, fileBuffer);

    close(sock);
}

void dataHandler(int sock, char *buffer, char *fileBuffer)
{
    // receive instruction from the server
    getData(sock, buffer);

    // Wait for the input from Client
    fgets(buffer, BUFFER_SIZE - 1, stdin);

    // Request for the file from the server
    sendData(sock, buffer);

    // Read file content received from the server
    getData(sock, fileBuffer);

    // Write file content to the folder
    writeFile(fileBuffer);
}

void getData(int sock, char *buffer)
{
    memset(buffer, 0, BUFFER_SIZE);
    int n = read(sock, buffer, BUFFER_SIZE - 1);
    if (n < 0)
    {
        printf("Error reading from the socket\n");
    }
    printf("%s\n", buffer);
}

void sendData(int sock, char *buffer)
{
    int n = write(sock, buffer, strlen(buffer));
    if (n < 0)
    {
        printf("Error writing to the socket\n");
    }
    //    printf("Client has entered %s\n", buffer);
}

void writeFile(char *fileBuffer)
{
    FILE *stream, *fopen();
    stream = fopen("hello.txt", "w");
    if (stream == NULL)
    {
        printf("Error in opening file %s\n", fileBuffer);
        exit(0);
    }
    fprintf(stream, "%s", fileBuffer);
    printf("writing file content: %s\n", fileBuffer);
    sleep(5);
    fclose(stream);
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