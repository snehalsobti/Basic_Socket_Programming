// This file includes the implementation of a TCP-based client in C
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
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socketFd == -1)
    {
        perror("Error while creating a socket");
        exit(EXIT_FAILURE);
    }
    
    return socketFd;
}

// Send Message to the server
void sendMessage(int socketFd, const char *message, int lenMessage)
{
	int writeSuccess = write(socketFd, (char *) message, lenMessage);
	
	if (writeSuccess == -1)
	{
		perror("Writing message to the server failed");
		exit(1);
	}
	
	printf("Message sent\n");
}

// Receive Response from the server
void receiveResponse(int socketFd, char *buffer, int lenBuffer)
{
    // Receiving response from server
	int readSuccess = read(socketFd, (char *) buffer, lenBuffer);
	
	if (readSuccess == -1)
	{
		perror("Reading response from the server failed");
		exit(1);
	}
	
	printf("Response from server: %s\n", buffer);
}

void closeSocket(int socketFd)
{
	close(socketFd);
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		perror("Port number and/or server address and/or message not provided");
		exit(1);
	}

	int portNumber = atoi(argv[2]);

	// Create a socket
	int socketFd = createSocket(portNumber);
	
	struct sockaddr_in serverAddr;
	
    // Initialize the data structure that will hold the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
   
    serverAddr.sin_family = AF_INET; // Address family -> IPv4
    serverAddr.sin_addr.s_addr = htonl(atoi(argv[1])); // IP address of server given by argv[1]
    serverAddr.sin_port = htons(portNumber);	
	
	// Make a connection to the server
	int connectSuccess = connect(socketFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	
	if (connectSuccess == -1)
	{
		perror("Connection to the server failed");
		exit(1);
	}
	
	const char *message = argv[3];
	
	// Send message to the server
	sendMessage(socketFd, (char *) message, strlen(message));
	
	char buffer[MAXSIZE];
	memset(buffer, 0, MAXSIZE);
	
	// Receive response from the server
	receiveResponse(socketFd, (char *) buffer, sizeof(buffer));
	
	closeSocket(socketFd);
}