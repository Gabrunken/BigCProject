#ifndef network_h_
#define network_h_
#include <stdint.h>
#include <winsock2.h>

#define BIGC_TCP 0
#define BIGC_UDP 1

typedef uint8_t bigc_NetworkProtocol;

//Returns FALSE if something went wrong
uint8_t bigc_network_InitializeModule();

//Starts a new thread, which the user controls defining the callback function, with serverSocket as argument
//I suggest to join the thread before closing the application. It's always good to close any sub-process
//That is running, so when you're done with your main loop, just join and wait for it to be done.
//Protocol is BIGC_UDP or BIGC_TCP
//ip is a string in this format: x.x.x.x --> i.e. 192.168.1.100
//The args in the callback function must be freed, the user must worry about this;
//the args is the server socket, used for sending and receiving data
//Also closing the socket is a mandatory task for the user when the connection needs to be closed
//Remember to shutdown() before closing the socket to notify the other host that you are quitting
//Or else the other could think an error occurred, like the other host crashed or something similar.
//For the UDP protocol this is kind of different, since there is no real connection occuring,
//So quitting would simply mean to stop sending messages. For this i suggest to look up on the internet
//Or ask ChatGPT if it is mandatory or not, shouldn't be.
//To know if it's actually connected, use the callback function, if it never runs, then it didn't connect to the server
void bigc_network_TryConnectToServer(void* (*callback)(void*), char ip[16], uint16_t port, bigc_NetworkProtocol protocol);

//Check any incoming connection request from a client, returns TRUE if there are any
uint8_t bigc_network_CheckConnectionRequests(SOCKET serverSocket);
//Accept the first connection request on the queue, returns the client's socket or INVALID_SOCKET if something went wrong
SOCKET bigc_network_AcceptFirstConnectionRequest(SOCKET serverSocket);

//Starts a new thread, which the user controls defining the callback function, with serverSocket as argument
//I suggest to join the thread before closing the application. It's always good to close any sub-process
//That is running, so when you're done with your main loop, just join and wait for it to be done.
//Protocol is BIGC_UDP or BIGC_TCP
//The args in the callback function must be freed, the user must worry about this;
//the args is the server socket, used for sending and receiving data
//Also closing the socket is a mandatory task for the user when the connection needs to be closed
//Remember to shutdown() before closing the socket to notify the other host that you are quitting
//Or else the other could think an error occurred, like the other host crashed or something similar.
//For the UDP protocol this is kind of different, since there is no real connection occuring,
//So quitting would simply mean to stop sending messages. For this i suggest to look up on the internet
//Or ask ChatGPT if it is mandatory or not, shouldn't be.
//To know if it's actually connected, use the callback function, if it never runs, then it didn't connect to the server
uint8_t bigc_network_StartLocalServer(void* (*callback)(void*), uint16_t port, bigc_NetworkProtocol protocol);

//Check incoming data from a socket, if there is data to read, returns TRUE, otherwise FALSE
//Does EXACTLY as the "CheckConnectionRequests" function, instead this works with other hosts sending data,
//so the SOCKET is never the server.
//It would work as that CheckConnectionRequests function only if you put as the socket the server's.
uint8_t bigc_network_CheckIncomingData(SOCKET senderSocket);

#endif