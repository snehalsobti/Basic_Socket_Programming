# Basic Socket Programming #
UDP-based and TCP-based implementations of server and client in C Programming Language. These are UNIX-based --> for Windows, there is a need to change the header files, functions and data structures being used in the programs.
## How to compile ##
In the terminal, go to the directory containing the source code files and type the following :-   
``` gcc Server.c -o Server ```   
``` gcc Client.c -o Client ```   
where Server and Client are the names of executable files that will be created after compilation -> you can keep any name
## How to Run the Programs ##
* Open two terminals on a UNIX/Linux computer (you may use a VM or WSL if you have a Windows PC/Laptop). One terminal will act as server and the other will act as a client.  
* Go to the directory containing the executable files of the server and the client programs
* Type the following on one terminal :-         ``` ./Server 8080 15000 ```   
      _where 8080 is the port number and 15000 is the timeout in milliseconds_  
* Type the following on the other terminal :-     ``` ./Client loopback 8080 Okay ```    
      _where loopback is the server address (it is the local machine i.e 127.0.0.1), 8080 is the port number and Okay is the message to be sent to the client_   
* ___To know more --> check the documentation (comments) in the source code files___

