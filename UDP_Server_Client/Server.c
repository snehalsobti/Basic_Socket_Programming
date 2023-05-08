// This file includes the implementation of a UDP-based server in C
// It is UNIX-based server --> For Windows-based server, need to use different headers, functions and data structures
// There are two command-line arguments to be passed for the program --> Port Number of the server and Timeout
// Timeout is the maximum time (in milliseconds) that the server will wait for the response, otherwise it shuts down
// The server does not wait for the response indefinitely

#include <stdlib.h> // General functions such as exit()
#include <stdio.h> // I/O related functions such as printf(), perror()
#include <netinet/in.h> // Data structures such as sockaddr and sockaddr_in
#include <sys/socket.h> // Socket programming related functions such as socket(), bind()
#include <sys/types.h> // Data types related to system calls
#include <sys/time.h> // Data structures such as timeval
#include <sys/select.h> // Functions such as select()
#include <unistd.h> // Miscellaneous types, symbolic constants and functions such as close()
#include <string.h> // String related functions such as strlen()

#define MAXSIZE 1024 // Maximum size of string for our purpose

// Return the socket file desciptor created by socket()
// Creates the socket and binds the socket file descriptor to the server address
int createServer(int portNumber, struct sockaddr_in *serverAddr)
{
	if (portNumber < 0)
	{
		perror("Wrong port number provided");
		exit(EXIT_FAILURE);
	}
	
	int socketFd;   // to store the socket file descriptor

	// Create a socket
	socketFd = socket(AF_INET, SOCK_DGRAM, 0);

	if (socketFd == -1)
	{
		perror("Error while creating a socket");
		exit(EXIT_FAILURE);
	}

	// Initialize the data structure that will hold the server address
	memset(serverAddr, 0, sizeof(*serverAddr));

	// htonl and htons stand for host byte order to network byte order long (32-bit) and short (16-bit)
	// respectively
	
	(*serverAddr).sin_family = AF_INET; // Address family -> IPv4
	(*serverAddr).sin_addr.s_addr = htonl(INADDR_ANY); // Address to accept any incoming messages
	(*serverAddr).sin_port = htons(portNumber);   
	
	// Bind the socket with server address
	int bindSuccess = bind(socketFd, (struct sockaddr*) serverAddr, sizeof(*serverAddr));

	if (bindSuccess == -1)
	{
		perror("Error while binding the socket with server address");
		exit(EXIT_FAILURE);
	}
	
	return socketFd;
}

// Return -1 if timeout occurred while receiving the data from the client
// Otherwise return the number of bytes received from the client
// timeout is in milliseconds
// buffer holds the message received from the client
int receiveMessage(int socketFd, char *buffer, int sizeOfBuffer, int timeout, struct sockaddr_in *clientAddr, int *sizeClientAddr)
{
	if (timeout < 0)
	{
		perror("Wrong timeout provided");
		exit(EXIT_FAILURE);
	}
	
	// readSet holds the socket file descriptors to be monitored for readability
	fd_set readSet;
	
	// tv holds the timeout in seconds and microseconds
	struct timeval tv;
	
	FD_ZERO(&readSet); // clear the readSet
	FD_SET(socketFd, &readSet); // set the value of socketFd to 1 in readSet
	
	tv.tv_sec = timeout / 1000; // seconds
	tv.tv_usec = (timeout % 1000) * 1000; // microseconds
	
	// select() blocks until either one of the file descriptors of readSet is ready for readability
	// or timeout specified in tv has run out
	int selectResult = select(socketFd + 1, &readSet, NULL, NULL, &tv);
	
	if (selectResult == -1)
	{
		perror("Error while waiting for data");
		exit(EXIT_FAILURE);
	}
	else if (selectResult == 0)
	{
		printf("Timeout occurred\n");
		return -1;
	}
	else
	{
		int numBytesRec;

		// Receiving message from client
		numBytesRec = recvfrom(socketFd, (char *) buffer, sizeOfBuffer - 1, MSG_WAITALL, 
		    (struct sockaddr *)clientAddr, sizeClientAddr);

		if (numBytesRec == -1)
		{
			perror("Error while receiving the message from client");
			return -1;
		}

		buffer[numBytesRec] = '\0';

		printf("Message received: %s\n", buffer);
		
		return numBytesRec;			
	}
}

// Send the response to the client
// Client Address is the same as the one that receiveMessage fills in
void sendResponse(int socketFd, char *response, int sizeOfResponse, struct sockaddr_in *clientAddr, int sizeClientAddr)
{		
	sendto(socketFd, (const char *) response, sizeOfResponse, MSG_CONFIRM, (struct sockaddr *) clientAddr, sizeClientAddr);

	printf("Response sent\n");	
}

void closeServer(int socketFd)
{
	close(socketFd);
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		perror("No timeout and/or port numbeer provided");
		exit(EXIT_FAILURE);
	}

	int socketFd;   // to store the socket file descriptor
	struct sockaddr_in serverAddr; // to store the server address

	// Create a server
	socketFd = createServer(atoi(argv[1]), &serverAddr); // argv[1] is the port number

	// Receiving and sending the message on loop
	// There is 1 server but can have multiple clients
	while (1)
	{
	    struct sockaddr_in clientAddr; // to store client address
	    
	    int sizeClientAddr = sizeof(clientAddr);
	    
	    // Initialize the data structure that will hold the client address
	    memset(&clientAddr, 0, sizeClientAddr);	    
	    
	    int numBytesRec;
	    char buffer[MAXSIZE];    
	    
	    // Receive the message from client
	    numBytesRec = receiveMessage(socketFd, (char *) buffer, MAXSIZE,
	   				atoi(argv[2]), &clientAddr, &sizeClientAddr); // argv[2] is the timeout in milliseconds
	    
	    if (numBytesRec == -1)
	    {
	    	printf("Shutting down the server\n");
	    	closeServer(socketFd);
	    	return 0;
	    }
	    
	    const char *response = "Hello, I am the server";
	    
	    // Send response to the client
	    sendResponse(socketFd, (char *) response, strlen(response), &clientAddr, sizeClientAddr);
	}

	// Close the server
	closeServer(socketFd);
    
    return 0;
}


