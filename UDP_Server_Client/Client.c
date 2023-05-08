// This file includes the implementation of a UDP-based client in C
// It is UNIX-based client --> For Windows-based client, need to use different headers, functions and data structures
// There are three command-line arguments to be passed for the program --> 
// Server address, Port Number of the server, and Message to be sent to the client
// If local machine is acting as server as well as client, use loopback as server address i.e 127.0.0.1 (IPv4)

#include <stdlib.h> // General functions such as exit()
#include <stdio.h> // I/O related functions such as printf(), perror()
#include <netinet/in.h> // Data structures such as sockaddr and sockaddr_in
#include <sys/socket.h> // Socket programming related functions such as socket(), bind()
#include <sys/types.h> // Data types related to system calls
#include <unistd.h> // Miscellaneous types, symbolic constants and functions such as close()
#include <string.h> // String related functions such as strlen()

#define MAXSIZE 1024 // Maximum size of string for our purpose

// Return the socket file desciptor created by socket()
// Creates the socket
int createSocket(int portNumber)
{
    int socketFd;   // to store the socket file descriptor
    
    // Create a socket
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (socketFd == -1)
    {
        perror("Error while creating a socket");
        exit(EXIT_FAILURE);
    }
    
    return socketFd;
}

// Send Message to the server
void sendMessage(int socketFd, const char *message, int lenMessage, struct sockaddr_in *serverAddr)
{
    sendto(socketFd, message, lenMessage, MSG_CONFIRM, (struct sockaddr *)serverAddr, sizeof(*serverAddr));
     	
    printf("Message sent\n");
}

// Receive Response from the server
void receiveResponse(int socketFd, char *buffer, int lenBuffer, struct sockaddr_in *serverAddr, int *sizeServerAddr)
{
    // Receiving response from server
    int numBytesRec = recvfrom(socketFd, (char *) buffer, lenBuffer, MSG_WAITALL, (struct sockaddr *)serverAddr, sizeServerAddr);
    
    if (numBytesRec == -1)
    {
        perror("Error while receiving the response from client");
        exit(EXIT_FAILURE);
    }
    
    buffer[numBytesRec] = '\0';
    
    printf("Response received: %s\n", buffer);
}

void closeSocket(int socketFd)
{
	close(socketFd);
}

int main(int argc, char* argv[]) {
    
    if (argc < 4)
    {
        perror("No message and/or server address and/or port number provided");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in serverAddr; // To store server address
    int portNumber = atoi(argv[2]);
    
    // Create a socket
    int socketFd = createSocket(portNumber);

    // Initialize the data structure that will hold the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
   
    serverAddr.sin_family = AF_INET; // Address family -> IPv4
    serverAddr.sin_addr.s_addr = htonl(atoi(argv[1])); // IP address of server given by argv[1]
    serverAddr.sin_port = htons(portNumber);	 

    const char *message = argv[3];
    
    // Send message to the server
    sendMessage(socketFd, (const char *) message, strlen(message), &serverAddr);
    
    char buffer[MAXSIZE];
    int sizeServerAddr = sizeof(serverAddr);
    
    // Receive response from the server
    receiveResponse(socketFd, (char *) buffer, MAXSIZE - 1, &serverAddr, &sizeServerAddr);
    
    closeSocket(socketFd);
}