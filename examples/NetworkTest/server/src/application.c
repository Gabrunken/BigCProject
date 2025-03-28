#include <application.h>
#include <bigc.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENTS 100

uint8_t closeServer = BIGC_FALSE;
pthread_t serverThreadID;

void* ServerCallback(void* args)
{
	serverThreadID = pthread_self();

	SOCKET serverSocket = *(SOCKET*)args;

	SOCKET clientSockets[MAX_CLIENTS] = {};
	uint8_t connectedClients = 0;

	while(!closeServer)
	{
		while(bigc_network_CheckConnectionRequests(serverSocket))
		{
			SOCKET newClient = bigc_network_AcceptFirstConnectionRequest(serverSocket);
			if(newClient == INVALID_SOCKET)
			{
				printf("Failed to accept a client's request to connect.\n");
				continue;
			}

			//loop until a free client socket slot
			for(uint8_t i = 0; i < MAX_CLIENTS; i++)
			{
				if(clientSockets[i] == 0)
				{
					clientSockets[i] = newClient;
					printf("New client!\n");
					break;
				}
			}

			connectedClients++;
		}

		//here client recv and send logic (recv also occupies about the disconnection of a client)
		{
			char message[30] = {};

			for(uint8_t i = 0; i < MAX_CLIENTS; i++)
			{
				if(clientSockets[i] == 0) continue;
				if(bigc_network_CheckIncomingData(clientSockets[i]))
				{
					int recvResult = recv(clientSockets[i], message, sizeof(message), 0);
					if(recvResult == 0)
					{
						//The client disconnected
						printf("A client disconnected.\n");
						shutdown(clientSockets[i], SD_BOTH); //Telling the client we are going to close the connection with him
						closesocket(clientSockets[i]);
						clientSockets[i] = 0;
						connectedClients--;
					}

					else if(recvResult == SOCKET_ERROR)
					{
						printf("A client failed to send data or quitted without telling. Closing socket.\n");
						closesocket(clientSockets[i]);
						clientSockets[i] = 0;
						connectedClients--;
					}

					else
					{
						//Here he sent data
					}
				}
			}
		}
	}

	printf("Server shutting down...\n");

	//Tell the clients still connected we are closing the server... doesnt seem to work
	for(uint8_t i = 0; i < MAX_CLIENTS; i++)
	{
		if(clientSockets[i] == 0) continue;
		shutdown(clientSockets[i], SD_BOTH); //Telling the client we are going to close the connection with him
		closesocket(clientSockets[i]);
		connectedClients--;
	}

	closesocket(serverSocket);
	free(args);

	printf("Server closed\n");
}

void Initialize()
{
	//Basically when the program ends (at the final "return" statement), calls the CleanUp function, which just cleans up everything and frees memory.
	atexit(CleanUp);

	if(!bigc_network_InitializeModule())
	{
		printf("Something went wrong when initializing network module.\n");
		return;
	}

	bigc_network_StartLocalServer(ServerCallback, 42000, BIGC_TCP);
}

void Loop()
{
	char message[4];

	while(strcmp(message, "stop"))
	{
		fgets(message, 5, stdin); //5 because 1 is the newline char i think... or maybe the terminator char /0
	}

	getchar();
	closeServer = BIGC_TRUE;
	pthread_join(serverThreadID, NULL);
}

void CleanUp()
{
	#ifdef DEBUG
	printf("\nClean up done.\nPress ENTER to terminate the process.");
	getchar();
	#endif
}