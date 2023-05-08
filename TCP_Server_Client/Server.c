// This file includes the implementation of a TCP-based server in C
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

// Return the socket file descriptor created by socket() function
// It creates the socket, binds it with the server address and then, leaves the server in passive mode
// It is ready for listening to incoming connections
int createServer(int portNumber, struct sockaddr_in *serverAddr)
{
	int socketFd; // to store the socket file descriptor
	
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (socketFd == -1)
	{
		perror("Socket creation failed");
		exit(1);	
	}
	
	int optval = 1;
	
	// SO_REUSEADDR allows the same socket to be used for another connection
	// even if it has not been closed yet
	// We enable this option because even after we call the close() function,
	// it takes a few seconds to actually close the socket (it goes into TIME_WAIT state)
	setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	
	// Initialize the data structure that will store the server address
	memset(serverAddr, 0, sizeof(*serverAddr));	
	
	// htonl and htons stand for host byte order to network byte order long (32-bit) and short (16-bit)
	// respectively
	
	(*serverAddr).sin_family = AF_INET; // Address family -> IPv4
	(*serverAddr).sin_addr.s_addr = htonl(INADDR_ANY); // Address to accept any incoming messages
	(*serverAddr).sin_port = htons(portNumber);
	
	// Bind the socket with server address
	int bindSuccess = bind(socketFd, (struct sockaddr *) serverAddr, sizeof(*serverAddr));
	
	if (bindSuccess == -1)
	{
		perror("Binding failed");
		exit(1);
	}
	
	int backlog = 5; // Maximum number of connections to be kept in queue by listen()
	
	// Put the server in passive mode, ready to listen to the incoming connections
	int listenSuccess = listen(socketFd, backlog);	
	
	if (listenSuccess == -1)
	{
		perror("Listening failed");
		exit(1);
	}	
	
	return socketFd;
}

// Return -1 if timeout occurred
// Otherwise return the new socket file descriptor created for a particular connection (by accept())
int connectToClient(int socketFd, struct sockaddr_in * clientAddr, int * sizeClientAddr, int timeout)
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
		exit(1);
	}
	else if (selectResult == 0)
	{
		printf("Timeout occurred\n");
		return -1;
	}	
	else
	{
		int newSocketFd; // new file descriptor for the connection established
		
		newSocketFd = accept(socketFd, (struct sockaddr *) clientAddr, sizeClientAddr);
		
		if (newSocketFd == -1)
		{
			perror("Accepting a connection failed");
			exit(1);
		}
		
		return newSocketFd;
	}
}

// Return -1 if unsuccessful in receiving data
// Otherwise return number of bytes received
int receiveMessage(int newSocketFd, char * buffer, int sizeBuffer)
{
	return  read(newSocketFd, buffer, sizeBuffer);	
}

// Return -1 if unsuccessful in sending data
// Otherwise return number of bytes sent
int sendMessage(int newSocketFd, char * response, int sizeResponse)
{
	return  write(newSocketFd, response, sizeResponse);	
}

void closeServer(int socketFd)
{
	close(socketFd);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		perror("Timeout and/or port number not provided");
		exit(1);
	}	
	
	struct sockaddr_in serverAddr; // to store the server address
	int portNumber = atoi(argv[1]); // atoi() converts string to integer

	// Create the server
	int socketFd = createServer(portNumber, &serverAddr);

	int timeout = atoi(argv[2]); // timeout in milliseconds
	
	struct sockaddr_in clientAddr;
	int sizeClientAddr = sizeof(clientAddr);	
		
	// Accept an incoming connection from a client	
	int newSocketFd = connectToClient(socketFd, &clientAddr, &sizeClientAddr, timeout); 
	
	if (newSocketFd == -1)
	{
		printf("Connection took more time than the desired timeout.\n Shutting down the server.\n");
		return 0;
	}	
		
	char buffer[MAXSIZE];
	memset(buffer, 0, MAXSIZE);
	
	// Receive message from the client
	int readSuccess = receiveMessage(newSocketFd, (char *) buffer, MAXSIZE);
	
	if (readSuccess == -1)
	{
		perror("Reading message from the client failed");
		exit(1);
	}
	
	printf("Message from client: %s\n", buffer);
	
	const char *response = "Hello, I am the server";
	
	// Send response to the client
	int writeSuccess = sendMessage(newSocketFd, (char *) response, strlen(response));
	
	if (writeSuccess == -1)
	{
		perror("Writing response to the client failed");
		exit(1);
	}
	
	printf("Response sent\n");
	
	closeServer(socketFd);
	closeServer(newSocketFd);
}
